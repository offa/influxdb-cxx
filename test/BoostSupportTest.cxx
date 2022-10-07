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

#include "BoostSupport.h"
#include "InfluxDBException.h"
#include "mock/TransportMock.h"
#include <boost/property_tree/exceptions.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_version_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <date/date.h>
#include <sstream>

namespace influxdb::test
{
    TEST_CASE("With UDP returns transport", "[BoostSupportTest]")
    {
        CHECK(internal::withUdpTransport(http::url{}) != nullptr);
    }

    TEST_CASE("With Unix socket returns transport", "[BoostSupportTest]")
    {
        CHECK(internal::withUnixSocketTransport(http::url{}) != nullptr);
    }

    TEST_CASE("UDP transport throws on create database", "[BoostSupportTest]")
    {
        auto udp = internal::withUdpTransport(http::url{});
        CHECK_THROWS_AS(udp->createDatabase(), std::runtime_error);
    }

    TEST_CASE("UDP transport throws on proxy", "[BoostSupport]")
    {
        auto udp = internal::withUdpTransport(http::url{});
        CHECK_THROWS_AS(udp->setProxy(Proxy{"udp://should-throw"}), std::runtime_error);
    }

    TEST_CASE("Unix socket transport throws on create database", "[BoostSupportTest]")
    {
        auto unix = internal::withUnixSocketTransport(http::url{});
        CHECK_THROWS_AS(unix->createDatabase(), std::runtime_error);
    }

    TEST_CASE("Unix socket transport throws on proxy", "[BoostSupportTest]")
    {
        auto unix = internal::withUnixSocketTransport(http::url{});
        CHECK_THROWS_AS(unix->setProxy(Proxy{"unix:///tmp/should_throw"}), std::runtime_error);
    }

    TEST_CASE("Query is passed to transport", "[BoostSupportTest]")
    {
        TransportMock transport;
        REQUIRE_CALL(transport, query("SELECT * from test WHERE host = 'localhost'"))
            .RETURN(R"({"results":[{"statement_id":0}]})");

        internal::queryImpl(&transport, "SELECT * from test WHERE host = 'localhost'");
    }

    TEST_CASE("Query throws if transport throws", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).THROW(InfluxDBException{"unit test", "Intentional"});

        CHECK_THROWS_AS(internal::queryImpl(&transport, "select should throw"), InfluxDBException);
    }

    TEST_CASE("Query returns empty if empty result", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"({"results":[]})");

        CHECK(internal::queryImpl(&transport, "SELECT * from test").empty());
    }

    TEST_CASE("Query returns point of single result", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        std::istringstream in{"2021-01-01T00:11:22.123456789Z"};
        std::chrono::system_clock::time_point expectedTimeStamp{};
        in >> date::parse("%FT%T%Z", expectedTimeStamp);

        TransportMock transport;
        ALLOW_CALL(transport, query(_))
            .RETURN(R"({"results":[{"statement_id":0,)"
                    R"("series":[{"name":"unittest","columns":["time","host","value"],)"
                    R"("values":[["2021-01-01T00:11:22.123456789Z","localhost",112233]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        const auto point = result[0];
        CHECK(point.getName() == "unittest");
        CHECK(point.getTimestamp() == expectedTimeStamp);
        CHECK(point.getTags() == "host=localhost");
        CHECK(point.getFields() == "value=112233.000000000000000000");
    }

    TEST_CASE("Query returns points of multiple results", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_))
            .RETURN(R"({"results":[{"statement_id":0,)"
                    R"("series":[{"name":"unittest","columns":["time","host","value"],)"
                    R"("values":[["2021-01-01:11:22.000000000Z","host-0",100],)"
                    R"(["2021-01-01T00:11:23.560000000Z","host-1",30],)"
                    R"(["2021-01-01T00:11:24.780000000Z","host-2",54]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 3);
        CHECK(result[0].getName() == "unittest");
        CHECK(result[0].getTags() == "host=host-0");
        CHECK(result[0].getFields() == "value=100.000000000000000000");
        CHECK(result[1].getName() == "unittest");
        CHECK(result[1].getTags() == "host=host-1");
        CHECK(result[1].getFields() == "value=30.000000000000000000");
        CHECK(result[2].getName() == "unittest");
        CHECK(result[2].getTags() == "host=host-2");
        CHECK(result[2].getFields() == "value=54.000000000000000000");
    }

    TEST_CASE("Query throws on invalid result", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_))
            .RETURN(R"({"invalid-results":[]})");

        CHECK_THROWS_AS(internal::queryImpl(&transport, "SELECT * from test"), boost::property_tree::ptree_bad_path);
    }

    TEST_CASE("Query is safe to empty name", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_))
            .RETURN(R"({"results":[{"statement_id":0,"series":[{"columns":["time","host","value"],)"
                    R"("values":[["2021-01-01:11:22.000000000Z","x",8]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        CHECK(result[0].getName() == "");
        CHECK(result[0].getTags() == "host=x");
    }

    TEST_CASE("Query reads optional tags element", "[BoostSupportTest")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_))
            .RETURN(R"({"results":[{"statement_id":0,"series":[{"name":"x","tags":{"type":"sp"},"columns":["time","value"],)"
                    R"("values":[["2022-01-01:01:02.000000000ZZ",99]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        CHECK(result[0].getTags() == "type=sp");
    }
}
