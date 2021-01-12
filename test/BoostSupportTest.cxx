// MIT License
//
// Copyright (c) 2020 offa
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
#include <boost/property_tree/exceptions.hpp>
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    namespace
    {
        struct TransportMock : public Transport
        {
            void send([[maybe_unused]] std::string&& message) override
            {
                throw "Mock not implemented";
            }

            MAKE_MOCK1(query, std::string(const std::string&), override);
        };
    }

    TEST_CASE("With UDP returns transport", "[BoostSupportTest]")
    {
        CHECK(internal::withUdpTransport(http::url{}) != nullptr);
    }

    TEST_CASE("With Unix socket returns transport", "[BoostSupportTest]")
    {
        CHECK(internal::withUnixSocketTransport(http::url{}) != nullptr);
    }

    TEST_CASE("Query is passed to transport", "[BoostSupportTest]")
    {
        TransportMock transport;
        REQUIRE_CALL(transport, query("SELECT * from test WHERE host = 'localhost'")).RETURN(R"({"results":[{"statement_id":0}]})");

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

        TransportMock transport;
        ALLOW_CALL(transport, query(_))
            .RETURN(R"({"results":[{"statement_id":0,"series":[{"name":"unittest","columns":["time","host","value"],"values":[["2021-01-01:11:22.000000000Z","localhost",112233]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        const auto point = result[0];
        CHECK(point.getName() == "unittest");
        CHECK(std::chrono::duration_cast<std::chrono::seconds>(point.getTimestamp().time_since_epoch()).count() == 1609455600);
        CHECK(point.getTags() == "host=localhost");
        CHECK(point.getFields() == "value=112233.000000000000000000");
    }

    TEST_CASE("Query returns points of multiple results", "[BoostSupportTest]")
    {
        using namespace Catch::Matchers;
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_))
            .RETURN(R"({"results":[{"statement_id":0,"series":[{"name":"unittest","columns":["time","host","value"],"values":[["2021-01-01:11:22.000000000Z","host-0",100],["2021-01-01:11:23.000000000Z","host-1",30],["2021-01-01:11:24.000000000Z","host-2",54]]}]}]})");

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
            .RETURN(R"({"results":[{"statement_id":0,"series":[{"columns":["time","host","value"],"values":[["2021-01-01:11:22.000000000Z","x",8]]}]}]})");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        CHECK(result[0].getName() == "");
        CHECK(result[0].getTags() == "host=x");
    }

}
