// MIT License
//
// Copyright (c) 2020-2023 offa
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

#include <thread>
#include "SystemTest.h"

namespace influxdb::test
{
    namespace
    {
        std::size_t querySize(influxdb::InfluxDB& db, const std::string& measurement, const std::string& tag)
        {
            // Quote the measurement name to avoid escaped characters being incorrectly interpreted
            return db.query(R"(select * from ")" + measurement + R"(" where type=')" + tag + "'").size();
        }

        std::size_t querySize(influxdb::InfluxDB& db, const std::string& tag)
        {
            return querySize(db, "x", tag);
        }
    }

    TEST_CASE("InfluxDB system test", "[InfluxDBST]")
    {
        using namespace Catch::Matchers;

        auto db = configure("st_db");


        SECTION("Database does not exist")
        {
            const auto response = db->execute("show databases");
            CHECK_THAT(response, !ContainsSubstring(R"(["st_db"])"));
        }

        SECTION("Query on non existing database returns empty")
        {
            CHECK(db->query("select * from st_db").empty());
        }

        SECTION("Create database if not existing")
        {
            db->createDatabaseIfNotExists();
            CHECK_THAT(db->execute("show databases"), ContainsSubstring(R"(["st_db"])"));
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
            CHECK_THROWS_AS(db->query("select* from_INVALID"), InfluxDBException);
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
            CHECK_THROWS_AS(db->write(Point{"test,this=is ,,====,, invalid"}), InfluxDBException);
        }

        SECTION("Write to unreachable host throws")
        {
            auto invalidDb = influxdb::InfluxDBFactory::Get("http://non_existing_host:1234?db=not_existing_db");
            CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), InfluxDBException);
        }

        SECTION("Write to nonexistent database throws")
        {
            auto invalidDb = configure("not_existing_db");
            CHECK_THROWS_AS(invalidDb->write(Point{"test"}.addField("value", 10)), InfluxDBException);
        }

        SECTION("Cleanup")
        {
            db->execute("drop database st_db");
        }
    }

    TEST_CASE("String Element Escaping", "[InfluxDBST]")
    {
        using namespace Catch::Matchers;

        const std::string dbName{"st_escaped_db"};
        const std::string pointType{"escaped"};
        auto db = configure(dbName);

        // String elements which need to be escaped in line protocol
        const std::string unescapedMeasurementName{"esc, measurement"};
        const std::string unescapedTagKey{"esc,= tag"};
        const std::string unescapedTagValue{"esc,= value"};
        const std::string unescapedFieldKey{"esc,= field"};
        const std::string unescapedFieldValue{R"(esc"\value)"};

        SECTION("Create test database")
        {
            db->createDatabaseIfNotExists();
            CHECK_THAT(db->execute("show databases"), ContainsSubstring(dbName));
        }

        SECTION("Write point with escaped string elements")
        {
            db->createDatabaseIfNotExists();
            CHECK(querySize(*db, unescapedMeasurementName, pointType) == 0);
            db->write(Point{unescapedMeasurementName}.addTag(unescapedTagKey, unescapedTagValue).addField(unescapedFieldKey, unescapedFieldValue).addTag("type", pointType));
            CHECK(querySize(*db, unescapedMeasurementName, pointType) == 1);
        }

        SECTION("Queried point string elements should be unescaped")
        {
            const auto response{db->query(R"(select * from ")" + unescapedMeasurementName + R"(" where type=')" + pointType + "'")};
            CHECK(response.size() == 1);

            const Point& point{response.at(0)};

            // Measurement
            CHECK(point.getName() == unescapedMeasurementName);

            // Tags
            const Point::TagSet& tags{point.getTagSet()};
            CHECK(tags.size() == 3);
            // Should contain the unescaped tag key and value
            CHECK(tags.end() != std::find(tags.begin(), tags.end(), Point::TagSet::value_type{unescapedTagKey, unescapedTagValue}));
            CHECK(tags.end() != std::find(tags.begin(), tags.end(), Point::TagSet::value_type{"type", "escaped"}));
            // Queried string values actually end up in the tags (see queryImpl)
            CHECK(tags.end() != std::find(tags.begin(), tags.end(), Point::TagSet::value_type{unescapedFieldKey, unescapedFieldValue}));

            // Fields
            const Point::FieldSet& fields{point.getFieldSet()};
            // String fields are put in the tags (see above)
            CHECK(fields.size() == 0);
            db->execute("drop database " + dbName);
        }
    }

    TEST_CASE("UDP Transport", "[InfluxDBST]")
    {
        using namespace Catch::Matchers;

        const std::string dbName{"st_udp"};
        // UDP transport for writing points
        auto udpTransport{configure(dbName, std::nullopt, "udp", 8089)};
        // HTTP transport for running queries
        auto httpTransport{configure(dbName, std::nullopt, "http", 8086)};

        const std::string measurement{"x"};

        // Configured via INFLUXDB_UDP_BATCH_TIMEOUT in systemtest.yml
        static constexpr std::chrono::milliseconds INFLUXDB_BATCH_TIMEOUT{10};
        auto WaitForUDPBatchTimeout{[]()
                                    {
                                        // Make sure the batch is flushed
                                        std::this_thread::sleep_for(10 * INFLUXDB_BATCH_TIMEOUT);
                                    }};

        SECTION("Create database")
        {
            httpTransport->createDatabaseIfNotExists();
        }

        SECTION("Write single point")
        {
            CHECK(querySize(*httpTransport, "sp") == 0);
            udpTransport->write(Point{measurement}.addField("n", 0).addTag("type", "sp"));
            WaitForUDPBatchTimeout();
            CHECK(querySize(*httpTransport, "sp") == 1);
        }

        SECTION("UDP batch flush is aware of UDP packet size limit")
        {
            // 1KB string
            static const std::string kiloStr(std::size_t{1024}, 'k');

            CHECK(querySize(*httpTransport, "udp_batching") == 0);
            // Add 64 points with 1KB string (too large for a single UDP packet)
            constexpr std::size_t numPoints{64};
            // Enable batching (enough that all points will be written in a single batch)
            udpTransport->batchOf(numPoints + 1);
            for (std::size_t i{0}; i < numPoints; ++i)
            {
                udpTransport->write(Point{measurement}.addField("f" + std::to_string(i), kiloStr).addTag("type", "udp_batching"));
            }
            // Force flush
            udpTransport->flushBatch();
            // Check all points are written
            WaitForUDPBatchTimeout();
            CHECK(querySize(*httpTransport, "udp_batching") == numPoints);
        }

        SECTION("Cleanup")
        {
            httpTransport->execute("drop database " + dbName);
        }
    }
}
