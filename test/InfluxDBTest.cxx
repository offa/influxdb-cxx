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

#include "InfluxDB.h"
#include "InfluxDBException.h"
#include "mock/TransportMock.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    namespace
    {
        constexpr std::chrono::time_point<std::chrono::system_clock> ignoreTimestamp(std::chrono::milliseconds(4567));
    }

    TEST_CASE("Ctor throws on nullptr transport", "[InfluxDBTest]")
    {
        CHECK_THROWS_AS(InfluxDB{nullptr}, InfluxDBException);
    }

    TEST_CASE("Write transmits point", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, send("p f0=71i 4567000000"));

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.write(Point{"p"}.addField("f0", 71).setTimestamp(ignoreTimestamp));
    }

    TEST_CASE("Write transmits points", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, send("p0 f0=0i 4567000000\np1 f1=1i 4567000000\np2 f2=2i 4567000000"));

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.write({Point{"p0"}.addField("f0", 0).setTimestamp(ignoreTimestamp),
                  Point{"p1"}.addField("f1", 1).setTimestamp(ignoreTimestamp),
                  Point{"p2"}.addField("f2", 2).setTimestamp(ignoreTimestamp)});
    }

    TEST_CASE("Write adds global tags", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, send("p0,x=1 f0=11i 4567000000\n"
                                 "p1,x=1,existing=yes f1=22i 4567000000\n"
                                 "p2,x=1 f2=33i 4567000000"));
        REQUIRE_CALL(*mock, send("p4,x=1 f3=44i 4567000000"));
        REQUIRE_CALL(*mock, send("p5,x=1 f4=55i 4567000000"));

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.addGlobalTag("x", "1");
        db.write({Point{"p0"}.addField("f0", 11).setTimestamp(ignoreTimestamp),
                  Point{"p1"}.addField("f1", 22).addTag("existing", "yes").setTimestamp(ignoreTimestamp),
                  Point{"p2"}.addField("f2", 33).setTimestamp(ignoreTimestamp)});
        db.write(Point{"p4"}.addField("f3", 44).setTimestamp(ignoreTimestamp));
        db.batchOf(1);
        db.write(Point{"p5"}.addField("f4", 55).setTimestamp(ignoreTimestamp));
    }

    TEST_CASE("Write with batch enabled adds point to batch if size not reached", "[InfluxDBTest]")
    {
        using trompeloeil::_;

        auto mock = std::make_shared<TransportMock>();

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.batchOf(3);
        db.write(Point{"x"});

        ALLOW_CALL(*mock, send(_));
        db.flushBatch();
    }

    TEST_CASE("Write with batch enabled writes points if size reached", "[InfluxDBTest]")
    {
        using trompeloeil::_;

        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, send("x 4567000000\ny 4567000000\nz 4567000000"));

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.batchOf(3);
        db.write(Point{"x"}.setTimestamp(ignoreTimestamp));
        db.write({Point{"y"}.setTimestamp(ignoreTimestamp),
                  Point{"z"}.setTimestamp(ignoreTimestamp),
                  Point{"not-transmitted"}.setTimestamp(ignoreTimestamp)});

        ALLOW_CALL(*mock, send(_));
        db.flushBatch();
    }

    TEST_CASE("Flush batch transmits pending points", "[InfluxDBTest]")
    {
        using trompeloeil::_;

        auto mock = std::make_shared<TransportMock>();

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.batchOf(300);
        db.write(Point{"x"}.setTimestamp(ignoreTimestamp));
        db.write({Point{"y"}.setTimestamp(ignoreTimestamp),
                  Point{"z"}.setTimestamp(ignoreTimestamp)});

        REQUIRE_CALL(*mock, send("x 4567000000\ny 4567000000\nz 4567000000"));
        db.flushBatch();
    }

    TEST_CASE("Destructs cleanly with pending batches", "[InfluxDBTest]")
    {
        using trompeloeil::_;

        auto mock = std::make_shared<TransportMock>();

        {
            ALLOW_CALL(*mock, send(_)).THROW(std::runtime_error{"Intentional"});
            InfluxDB db{std::make_unique<TransportAdapter>(mock)};
            db.batchOf(100);
            db.write(Point{"x"}.setTimestamp(ignoreTimestamp));
        }
    }

    TEST_CASE("Flush batch does nothing if batch disabled", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        InfluxDB db{std::make_unique<TransportAdapter>(mock)};

        {
            REQUIRE_CALL(*mock, send("x 4567000000"));
            db.write(Point{"x"}.setTimestamp(ignoreTimestamp));
        }

        db.flushBatch();
    }

    TEST_CASE("Clear Batch clears batch", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.batchOf(10);

        db.write(Point{"x"}.setTimestamp(ignoreTimestamp));
        CHECK(db.batchSize() == 1);
        db.clearBatch();
        CHECK(db.batchSize() == 0);
    }

    TEST_CASE("Create database throws if unsupported by transport", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, createDatabase()).THROW(std::runtime_error{"Intentional"});

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        CHECK_THROWS_AS(db.createDatabaseIfNotExists(), InfluxDBException);
    }

    TEST_CASE("Create database creates database through transport", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, createDatabase());

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.createDatabaseIfNotExists();
    }
}
