// MIT License
//
// Copyright (c) 2020-2024 offa
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
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    namespace
    {
        http::url parse(std::string in)
        {
            return http::ParseHttpUrl(in);
        }
    }


    TEST_CASE("Parse url", "[UriParserTest]")
    {
        const std::string input{"https://xyz.com/fghi/jklm"};
        const auto url = parse(input);

        CHECK(url.protocol == "https");
        CHECK(url.user == "");
        CHECK(url.password == "");
        CHECK(url.host == "xyz.com");
        CHECK(url.path == "/fghi/jklm");
        CHECK(url.search == "");
        CHECK(url.url == input);
        CHECK(url.port == 0);
    }

    TEST_CASE("Parse protocol", "[UriParserTest]")
    {
        CHECK(parse("http://xyz.com").protocol == "http");
        CHECK(parse("https://xyz.com").protocol == "https");
        CHECK(parse("udp://xyz.com").protocol == "udp");
        CHECK(parse("unix://xyz.com").protocol == "unix");
    }

    TEST_CASE("Parse param", "[UriParserTest]")
    {
        CHECK(parse("http://xyz.com/aaa?param=value").search == "param=value");
    }

    TEST_CASE("Parse port", "[UriParserTest]")
    {
        CHECK(parse("http://xyz.com:12345").port == 12345);
    }

    TEST_CASE("Parse basic auth", "[UriParserTest]")
    {
        const auto url = parse("https://aaa:bbb@host0");
        CHECK(url.user == "aaa");
        CHECK(url.password == "bbb");
    }

    TEST_CASE("Parse auth token", "[UriParserTest]")
    {
        CHECK(parse("http://token@xyz.com").password == "token");
    }

}
