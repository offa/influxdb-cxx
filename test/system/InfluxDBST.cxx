#include "InfluxDBFactory.h"
#include "HTTP.h"
#include <catch2/catch.hpp>

TEST_CASE("InfluxDB system test", "[InfluxDBST]")
{
    using namespace influxdb;
    using namespace influxdb::transports;
    using namespace Catch::Matchers;

    const std::string url{"http://localhost:8086?db=st_db"};
    auto db = InfluxDBFactory::Get(url);
    HTTP http{url};

    SECTION("Database does not exist")
    {
        const auto response = http.query("show databases");
        CHECK_THAT(http.query("show databases"), !Contains(R"(["st_db"])"));
    }

    SECTION("Query on non existing database returns empty")
    {
        CHECK(db->query("select * from st_db").size() == 0);
    }

    SECTION("Create database if not existing")
    {
        db->createDatabaseIfNotExists();
        CHECK_THAT(http.query("show databases"), Contains(R"(["st_db"])"));
    }

    SECTION("Create database if not existing does nothing on existing database")
    {
        db->createDatabaseIfNotExists();
    }

    SECTION("Created database is empty")
    {
        CHECK(db->query("select * from st_db").size() == 0);
    }

    SECTION("Write point")
    {
        auto querySize = [&db] {
            return db->query(R"(select * from x where type='sp')").size();
        };

        CHECK(querySize() == 0);
        db->write(Point{"x"}.addField("value", 20).addTag("type", "sp"));
        CHECK(querySize() == 1);
    }

    SECTION("Query point")
    {
        const auto response = db->query("select * from x");
        CHECK(response.size() == 1);
        CHECK(response[0].getName() == "x");
        CHECK(response[0].getFields() == "value=20.000000000000000000");
        CHECK(response[0].getTags() == "type=sp");
    }

    SECTION("Write multiple points")
    {
        auto querySize = [&db] {
            return db->query(R"(select * from x where type='mp')").size();
        };

        CHECK(querySize() == 0);
        db->write(Point{"x"}.addField("n", 0).addTag("type", "mp"));
        CHECK(querySize() == 1);
        db->write(Point{"x"}.addField("n", 1).addTag("type", "mp"));
        CHECK(querySize() == 2);
        db->write(Point{"x"}.addField("n", 2).addTag("type", "mp"));
        CHECK(querySize() == 3);
        db->write(Point{"x"}.addField("n", 2).addTag("type", "mp"));
        CHECK(querySize() == 4);
    }

    SECTION("Write multiple points by container")
    {
        auto querySize = [&db] {
            return db->query(R"(select * from x where type='mpc')").size();
        };

        CHECK(querySize() == 0);

        db->write({Point{"x"}.addField("n", 0).addTag("type", "mpc"),
                   Point{"x"}.addField("n", 1).addTag("type", "mpc"),
                   Point{"x"}.addField("n", 2).addTag("type", "mpc")});

        CHECK(querySize() == 3);
    }

    SECTION("Query points")
    {
        const auto response = db->query(R"(select * from x where type='mpc')");
        CHECK(response.size() == 3);
        CHECK(response[0].getName() == "x");
        CHECK(response[0].getFields() == "n=0.000000000000000000");
        CHECK(response[1].getName() == "x");
        CHECK(response[1].getFields() == "n=1.000000000000000000");
        CHECK(response[2].getName() == "x");
        CHECK(response[2].getFields() == "n=2.000000000000000000");
    }

    SECTION("Write as batch doesn't send if batch size not reached")
    {
        auto querySize = [&db] {
            return db->query(R"(select * from x where type='bpns')").size();
        };

        db->batchOf(3);

        CHECK(querySize() == 0);
        db->write({Point{"x"}.addField("n", 0).addTag("type", "bpns"),
                   Point{"x"}.addField("n", 1).addTag("type", "bpns")});
        CHECK(querySize() == 0);
    }

    SECTION("Write as batch sends if batch size reached")
    {
        auto querySize = [&db] {
            return db->query(R"(select * from x where type='bp')").size();
        };

        db->batchOf(2);

        CHECK(querySize() == 0);
        db->write({Point{"x"}.addField("n", 1).addTag("type", "bp"),
                   Point{"x"}.addField("n", 2).addTag("type", "bp"),
                   Point{"x"}.addField("n", -1).addTag("type", "bp")});
        CHECK(querySize() == 2);
    }

    SECTION("Write as batch sends if on flush")
    {
        auto querySize = [&db] {
            return db->query(R"(select * from x where type='bpf')").size();
        };

        db->batchOf(200);

        CHECK(querySize() == 0);
        db->write({Point{"x"}.addField("n", 1).addTag("type", "bpf"),
                   Point{"x"}.addField("n", 2).addTag("type", "bpf"),
                   Point{"x"}.addField("n", -1).addTag("type", "bpf")});
        CHECK(querySize() == 0);
        db->flushBatch();
        CHECK(querySize() == 3);
    }

    SECTION("Cleanup")
    {
        http.query("drop database st_db");
    }
}
