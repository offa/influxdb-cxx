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

#include "InfluxDB.h"
#include "InfluxDBException.h"
#include "LineProtocol.h"
#include "mock/TransportMock.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    namespace
    {
        constexpr std::chrono::time_point<std::chrono::system_clock> ignoreTimestamp(std::chrono::milliseconds(4567));
        constexpr std::size_t unlimitedMessageSize{(std::numeric_limits<std::size_t>::max)()};
    }

    TEST_CASE("Ctor throws on nullptr transport", "[InfluxDBTest]")
    {
        CHECK_THROWS_AS(InfluxDB{nullptr}, InfluxDBException);
    }

    TEST_CASE("Write transmits point", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, getMaxMessageSize()).RETURN(unlimitedMessageSize);
        REQUIRE_CALL(*mock, send("p f0=71i 4567000000"));

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.write(Point{"p"}.addField("f0", 71).setTimestamp(ignoreTimestamp));
    }

    TEST_CASE("Write throws when point too large", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, getMaxMessageSize()).RETURN(1);

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        CHECK_THROWS_AS(db.write(Point{"p"}.addField("f0", 71).setTimestamp(ignoreTimestamp)), InfluxDBException);
    }

    TEST_CASE("Write transmits points", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, getMaxMessageSize()).RETURN(unlimitedMessageSize);
        REQUIRE_CALL(*mock, send("p0 f0=0i 4567000000\np1 f1=1i 4567000000\np2 f2=2i 4567000000"));

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.write({Point{"p0"}.addField("f0", 0).setTimestamp(ignoreTimestamp),
                  Point{"p1"}.addField("f1", 1).setTimestamp(ignoreTimestamp),
                  Point{"p2"}.addField("f2", 2).setTimestamp(ignoreTimestamp)});
    }

    TEST_CASE("Write vector transmits all possible points", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        const std::string firstPoint{"p0 f0=0i 4567000000"};
        const std::string secondPoint{"p1 f1=1i 4567000000"};
        const std::string fourthPoint{"p3 f3=3i 4567000000"};

        // Make transport max message size just large enough for one point
        CHECK(firstPoint.size() == secondPoint.size());
        CHECK(secondPoint.size() == fourthPoint.size());
        REQUIRE_CALL(*mock, getMaxMessageSize()).RETURN(firstPoint.size());
        // Expect the points which fit in the message size to be sent
        // i.e.: skip the third point which is too large
        REQUIRE_CALL(*mock, send(std::string{firstPoint}));
        REQUIRE_CALL(*mock, send(std::string{secondPoint}));
        REQUIRE_CALL(*mock, send(std::string{fourthPoint}));

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        // Expect write to throw because the third point exceeds the max message size
        CHECK_THROWS_AS(db.write({Point{"p0"}.addField("f0", 0).setTimestamp(ignoreTimestamp),
                                  Point{"p1"}.addField("f1", 1).setTimestamp(ignoreTimestamp),
                                  Point{"p2"}.addField("exceedlimit", true).setTimestamp(ignoreTimestamp),
                                  Point{"p3"}.addField("f3", 3).setTimestamp(ignoreTimestamp)}),
                        InfluxDBException);
    }

    TEST_CASE("Write adds global tags", "[InfluxDBTest]")
    {
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, send(R"(p0,x=1,t\,\=\ =v\,\=\  f0=11i 4567000000)"
                                 "\n"
                                 R"(p1,x=1,t\,\=\ =v\,\=\ ,existing=yes f1=22i 4567000000)"
                                 "\n"
                                 R"(p2,x=1,t\,\=\ =v\,\=\  f2=33i 4567000000)"));
        REQUIRE_CALL(*mock, send(R"(p4,x=1,t\,\=\ =v\,\=\  f3=44i 4567000000)"));
        REQUIRE_CALL(*mock, send(R"(p5,x=1,t\,\=\ =v\,\=\  f4=55i 4567000000)"));
        REQUIRE_CALL(*mock, getMaxMessageSize()).TIMES(3).RETURN(unlimitedMessageSize);

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.addGlobalTag("x", "1");
        // Special characters in global tags and values should be escaped
        db.addGlobalTag("t,= ", "v,= ");
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

        REQUIRE_CALL(*mock, getMaxMessageSize()).RETURN(unlimitedMessageSize);
        ALLOW_CALL(*mock, send(_));
        db.flushBatch();
    }

    TEST_CASE("Write with batch enabled writes points if size reached", "[InfluxDBTest]")
    {
        using trompeloeil::_;

        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, getMaxMessageSize()).TIMES(2).RETURN(unlimitedMessageSize);
        REQUIRE_CALL(*mock, send("x 4567000000\ny 4567000000\nz 4567000000"));

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.batchOf(3);
        db.write(Point{"x"}.setTimestamp(ignoreTimestamp));
        db.write({Point{"y"}.setTimestamp(ignoreTimestamp),
                  Point{"z"}.setTimestamp(ignoreTimestamp),
                  Point{"not-transmitted"}.setTimestamp(ignoreTimestamp)});

        REQUIRE_CALL(*mock, send("not-transmitted 4567000000"));
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

        REQUIRE_CALL(*mock, getMaxMessageSize()).RETURN(unlimitedMessageSize);
        REQUIRE_CALL(*mock, send("x 4567000000\ny 4567000000\nz 4567000000"));
        db.flushBatch();
    }

    TEST_CASE("Flush batch still clears batch on transmission error", "[InfluxDBTest]")
    {
        using trompeloeil::_;
        auto mock = std::make_shared<TransportMock>();

        auto MakePoint{[](const std::string& name) -> Point
                       {
                           Point p{name};
                           p.setTimestamp(ignoreTimestamp);
                           return p;
                       }};
        auto FormatPoint{[](const Point& p) -> std::string
                         {
                             LineProtocol formatter;
                             return formatter.format(p);
                         }};

        Point p1{MakePoint("p1")}, p2{MakePoint("p2")}, p4{MakePoint("p4")};
        std::string p1p2Line{FormatPoint(p1) + "\n" + FormatPoint(p2)};
        std::string p4Line{FormatPoint(p4)};
        // Set transport max message size to accommodate three test points (with newline delimiters)
        const std::size_t maxMessageSize{(p4Line.size() * 3) + 2};
        //
        REQUIRE_CALL(*mock, getMaxMessageSize()).TIMES(2).RETURN(maxMessageSize);
        // Create a point which is too large to be transmitted
        Point p3{MakePoint("p3")};
        p3.addField("exceedslimit", std::string(maxMessageSize + 1, 'x'));


        // Set batch size too large to auto-flush
        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        db.batchOf(300);

        // Add points to first batch via single and vector write methods
        db.write(std::move(p1));
        db.write({p2, p3, p4});

        // Flush batch and expect to write p1, p2 and p4 and throw on p3
        REQUIRE_CALL(*mock, send(std::move(p1p2Line)));
        REQUIRE_CALL(*mock, send(std::move(p4Line)));
        CHECK_THROWS_AS(db.flushBatch(), InfluxDBException);

        // Create points for second batch (max message size should be ok for these)
        Point p5{MakePoint("p5")}, p6{MakePoint("p6")}, p7{MakePoint("p7")};
        std::string p5p6p7Line{FormatPoint(p5) + "\n" + FormatPoint(p6) + "\n" + FormatPoint(p7)};

        // Write points to second batch via single and vector write methods
        db.write(std::move(p5));
        db.write({p6, p7});

        // Flush batch and check we only get the expected points and no exception
        REQUIRE_CALL(*mock, send(std::move(p5p6p7Line)));
        CHECK_NOTHROW(db.flushBatch());
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
            REQUIRE_CALL(*mock, getMaxMessageSize()).RETURN(unlimitedMessageSize);
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
        REQUIRE_CALL(*mock, createDatabase()).THROW(InfluxDBException{"Intentional"});

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

    TEST_CASE("Execute executes query", "[InfluxDBTest]")
    {
        const std::string response = "name: databases\nname\n----\n_internal\n";
        auto mock = std::make_shared<TransportMock>();
        REQUIRE_CALL(*mock, execute("show databases")).RETURN(response);

        InfluxDB db{std::make_unique<TransportAdapter>(mock)};
        const auto result = db.execute("show databases");
        CHECK(result == response);
    }
}
