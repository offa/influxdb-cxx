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

#include "LineProtocol.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

namespace influxdb::test
{
    using namespace Catch::Matchers;

    namespace
    {
        constexpr std::chrono::time_point<std::chrono::system_clock> ignoreTimestamp(std::chrono::milliseconds(54));
    }


    TEST_CASE("Empty measurement", "[LineProtocolTest]")
    {
        const auto point = Point{"p0"}.setTimestamp(ignoreTimestamp);
        const LineProtocol lineProtocol;
        CHECK_THAT(lineProtocol.format(point), Equals("p0 54000000"));
    }

    TEST_CASE("Measurement with value", "[LineProtocolTest]")
    {
        const auto point = Point{"p0"}.addField("f0", "1").setTimestamp(ignoreTimestamp);
        const LineProtocol lineProtocol;
        CHECK_THAT(lineProtocol.format(point), Equals(R"(p0 f0="1" 54000000)"));
    }

    TEST_CASE("Measurement with different types", "[LineProtocolTest]")
    {
        const auto point = Point{"multitype"}
                               .addField("int_value", 567)
                               .addField("longlong_value", 1234567890LL)
                               .addField("double_value", 123.4567)
                               .addField("string_value", "abc def ghi")
                               .setTimestamp(ignoreTimestamp);

        const LineProtocol lineProtocol;
        CHECK_THAT(lineProtocol.format(point), Matches("multitype "
                                                       "int_value=567i,"
                                                       "longlong_value=1234567890i,"
                                                       "double_value=123.45[0-9]*,"
                                                       "string_value=\"abc def ghi\""
                                                       " 54000000"));
    }

    TEST_CASE("Measurement with multiple values", "[LineProtocolTest]")
    {
        const auto point = Point{"multiFieldPoint"}
                               .addField("value0", 4455)
                               .addField("value1", 99807)
                               .addField("value2", 2334)
                               .setTimestamp(ignoreTimestamp);
        const LineProtocol lineProtocol;
        CHECK_THAT(lineProtocol.format(point), Equals("multiFieldPoint value0=4455i,"
                                                      "value1=99807i,"
                                                      "value2=2334i"
                                                      " 54000000"));
    }

    TEST_CASE("Measurement with tag", "[LineProtocolTest]")
    {
        const auto point = Point{"taggedPoint"}
                               .addField("x", 5)
                               .addTag("tag0", "value0")
                               .setTimestamp(ignoreTimestamp);
        const LineProtocol lineProtocol;
        CHECK_THAT(lineProtocol.format(point), Equals(R"(taggedPoint,tag0=value0 x=5i 54000000)"));
    }

    TEST_CASE("Measurement with multiple tags", "[LineProtocolTest]")
    {
        const auto point = Point{"taggedPoint"}
                               .addField("y", 9)
                               .addTag("t0", "v0")
                               .addTag("t1", "v1")
                               .addTag("t2", "v2")
                               .setTimestamp(ignoreTimestamp);
        const LineProtocol lineProtocol;
        CHECK_THAT(lineProtocol.format(point), Equals(R"(taggedPoint,t0=v0,t1=v1,t2=v2 y=9i 54000000)"));
    }

    TEST_CASE("Adds global tag", "[LineProtocolTest]")
    {
        const auto point = Point{"p0"}
                               .addField("n", 0)
                               .setTimestamp(ignoreTimestamp);
        const LineProtocol lineProtocol{"global=true"};
        CHECK_THAT(lineProtocol.format(point), Equals(R"(p0,global=true n=0i 54000000)"));
    }

    TEST_CASE("Adds global tag to existing tags", "[LineProtocolTest]")
    {
        const auto point = Point{"p0"}
                               .addField("n", 0)
                               .addTag("local", "1")
                               .setTimestamp(ignoreTimestamp);
        const LineProtocol lineProtocol{"global=true"};
        CHECK_THAT(lineProtocol.format(point), Equals(R"(p0,global=true,local=1 n=0i 54000000)"));
    }

    TEST_CASE("Adds multiple global tags", "[LineProtocolTest]")
    {
        const auto point = Point{"p1"}
                               .addField("n", 1)
                               .addTag("pointtag", "3")
                               .setTimestamp(ignoreTimestamp);
        const LineProtocol lineProtocol{"a=0,b=1,c=2"};
        CHECK_THAT(lineProtocol.format(point), Equals(R"(p1,a=0,b=1,c=2,pointtag=3 n=1i 54000000)"));
    }
}
