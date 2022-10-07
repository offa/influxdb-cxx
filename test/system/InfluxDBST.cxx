// MIT License
//
// Copyright (c) 2020-2022 offa
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "InfluxDBFactory.h"
#include "InfluxDBException.h"
#include "HTTP.h"
#include <cstdlib>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

namespace influxdb::test
{
    namespace
    {
        std::size_t querySize(influxdb::InfluxDB& db, const std::string& tag)
        {
            return db.query("select * from x where type='" + tag + "'").size();
        }

        std::string dbServerUrl()
        {
            if (const auto host = std::getenv("INFLUXDB_SYSTEMTEST_HOST"); host != nullptr)
            {
                return host;
            }
            return "localhost";
        }
    }

    TEST_CASE("InfluxDB system test", "[InfluxDBST]")
    {
        using namespace influxdb;
        using namespace influxdb::transports;
        using namespace Catch::Matchers;

        const std::string url{"http://" + dbServerUrl() + ":8086?db=st_db"};
        auto db = InfluxDBFactory::Get(url);
        HTTP http{url};


        SECTION("Database does not exist")
        {
            const auto response = http.query("show databases");
            CHECK_THAT(response, !ContainsSubstring(R"(["st_db"])"));
        }

        SECTION("Query on non existing database returns empty")
        {
            CHECK(db->query("select * from st_db").empty());
        }

        SECTION("Create database if not existing")
        {
            db->createDatabaseIfNotExists();
            CHECK_THAT(http.query("show databases"), ContainsSubstring(R"(["st_db"])"));
        }

        SECTION("Create database if not existing does nothing on existing database")
        {
            db->createDatabaseIfNotExists();
        }

        SECTION("Created database is empty")
        {
            CHECK(db->query("select * from st_db").empty());
        }

        SECTION("Write point")
        {
            CHECK(querySize(*db, "sp") == 0);
            db->write(Point{"x"}.addField("value", 20).addTag("type", "sp"));
            CHECK(querySize(*db, "sp") == 1);
        }

        SECTION("Query point")
        {
            const auto response = db->query("select * from x");
            CHECK(response.size() == 1);
            CHECK(response[0].getName() == "x");
            CHECK(response[0].getFields() == "value=20.000000000000000000");
            CHECK(response[0].getTags() == "type=sp");
        }

        SECTION("Query point with no matches")
        {
            const auto response = db->query("select * from nothing_to_find");
            CHECK(response.empty());
        }

        SECTION("Query point grouped by tag")
        {
            const auto response = db->query("select * from x group by type");
            CHECK(response.size() == 1);
            CHECK(response[0].getTags() == "type=sp");
        }

        SECTION("Query point throws on invalid query")
        {
            CHECK_THROWS_AS(db->query("select* from_INVALID"), BadRequest);
        }

        SECTION("Write multiple points")
        {
            CHECK(querySize(*db, "mp") == 0);
            db->write(Point{"x"}.addField("n", 0).addTag("type", "mp"));
            CHECK(querySize(*db, "mp") == 1);
            db->write(Point{"x"}.addField("n", 1).addTag("type", "mp"));
            CHECK(querySize(*db, "mp") == 2);
            db->write(Point{"x"}.addField("n", 2).addTag("type", "mp"));
            CHECK(querySize(*db, "mp") == 3);
            db->write(Point{"x"}.addField("n", 2).addTag("type", "mp"));
            CHECK(querySize(*db, "mp") == 4);
        }

        SECTION("Write multiple points by container")
        {
            CHECK(querySize(*db, "mpc") == 0);

            db->write({Point{"x"}.addField("n", 0).addTag("type", "mpc"),
                       Point{"x"}.addField("n", 1).addTag("type", "mpc"),
                       Point{"x"}.addField("n", 2).addTag("type", "mpc")});

            CHECK(querySize(*db, "mpc") == 3);
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
            db->batchOf(3);

            CHECK(querySize(*db, "bpns") == 0);
            db->write({Point{"x"}.addField("n", 0).addTag("type", "bpns"),
                       Point{"x"}.addField("n", 1).addTag("type", "bpns")});
            CHECK(querySize(*db, "bpns") == 0);
        }

        SECTION("Write as batch sends if batch size reached")
        {
            db->batchOf(2);

            CHECK(querySize(*db, "bp") == 0);
            db->write({Point{"x"}.addField("n", 1).addTag("type", "bp"),
                       Point{"x"}.addField("n", 2).addTag("type", "bp"),
                       Point{"x"}.addField("n", -1).addTag("type", "bp")});
            CHECK(querySize(*db, "bp") == 2);
        }

        SECTION("Write as batch sends if on flush")
        {
            db->batchOf(200);

            CHECK(querySize(*db, "bpf") == 0);
            db->write({Point{"x"}.addField("n", 1).addTag("type", "bpf"),
                       Point{"x"}.addField("n", 2).addTag("type", "bpf"),
                       Point{"x"}.addField("n", -1).addTag("type", "bpf")});
            CHECK(querySize(*db, "bpf") == 0);
            db->flushBatch();
            CHECK(querySize(*db, "bpf") == 3);
        }

        SECTION("Write of invalid line protocol throws")
        {
            CHECK_THROWS_AS(db->write(Point{"test,this=is ,,====,, invalid"}), BadRequest);
        }

        SECTION("Write to unreachable host throws")
        {
            auto invalidDb = influxdb::InfluxDBFactory::Get("http://non_existing_host:123456?db=not_existing_db");
            CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), ConnectionError);
        }

        SECTION("Write to nonexistent database throws")
        {
            auto invalidDb = influxdb::InfluxDBFactory::Get(url + "_nonexistent");
            CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), NonExistentDatabase);
        }

        SECTION("Cleanup")
        {
            http.query("drop database st_db");
        }
    }
}
