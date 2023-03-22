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

#include "UriParser.h"
#include "InfluxDBException.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    using http::ParseHttpUrl;

    TEST_CASE("Valid URI", "[UriParserTest]")
    {
        std::string urlString{"https://username:password@localhost:8086/path?search"};
        const auto url{ParseHttpUrl(urlString)};
        REQUIRE(url.protocol == "https");
        REQUIRE(url.user == "username");
        REQUIRE(url.password == "password");
        REQUIRE(url.host == "localhost");
        REQUIRE(url.port == 8086);
        REQUIRE(url.path == "/path");
        REQUIRE(url.search == "search");
    }

    TEST_CASE("Missing protocol", "[UriParserTest]")
    {
        std::string missingProtocol{"username:password@localhost:8086/path?search"};
        CHECK_THROWS_AS(ParseHttpUrl(missingProtocol), InfluxDBException);
    }

    TEST_CASE("No port", "[UriParserTest]")
    {
        std::string noPort{"http://localhost/db=dbname"};
        const auto url{ParseHttpUrl(noPort)};
        REQUIRE(url.protocol == "http");
        REQUIRE(url.user == "");
        REQUIRE(url.password == "");
        REQUIRE(url.host == "localhost");
        REQUIRE(url.port == http::url::PORT_NOT_SET);
        REQUIRE(url.path == "/db=dbname");
        REQUIRE(url.search == "");
    }

    TEST_CASE("Invalid port", "[UriParserTest]")
    {
        std::string colonButNoNumber{"udp://localhost:twelve"};
        CHECK_THROWS_AS(ParseHttpUrl(colonButNoNumber), InfluxDBException);
    }

    TEST_CASE("Minimum valid port", "[UriParserTest]")
    {
        std::string zeroPort{"udp://localhost:0"};
        const auto url{ParseHttpUrl(zeroPort)};
        REQUIRE(url.port == 0);
    }

    TEST_CASE("Maximum valid port", "[UriParserTest]")
    {
        std::string maxPort{"udp://localhost:65535"};
        const auto url{ParseHttpUrl(maxPort)};
        REQUIRE(url.port == 65535);
    }

    TEST_CASE("Out of range port", "[UriParserTest]")
    {
        std::string negativePort{"udp://localhost:-1"};
        CHECK_THROWS_AS(ParseHttpUrl(negativePort), InfluxDBException);
        std::string tooLargePort{"udp://localhost:65536"};
        CHECK_THROWS_AS(ParseHttpUrl(negativePort), InfluxDBException);
    }

}
