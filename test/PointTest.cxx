// MIT License
//
// Copyright (c) 2020-2025 offa
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

#include "InfluxDB/Point.h"
#include <limits>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>

namespace influxdb::test
{
    using namespace Catch::Matchers;

    TEST_CASE("Empty measurement", "[PointTest]")
    {
        const Point point{"test"};
        CHECK_THAT(point.getName(), Equals("test"));
        CHECK_THAT(point.getFields(), Equals(""));
        CHECK_THAT(point.getTags(), Equals(""));
    }

    TEST_CASE("Measurement with value", "[PointTest]")
    {
        const auto point = Point{"test"}.addField("field_name", "field_value");
        CHECK_THAT(point.getFields(), Equals(R"(field_name="field_value")"));
    }

    TEST_CASE("Measurement with values of different types", "[PointTest]")
    {
        const auto point = Point{"test"}
                               .addField("int_field", 3)
                               .addField("longlong_field", 1234LL)
                               .addField("string_field", "string value")
                               .addField("double_field", double{3.859})
                               .addField("bool_true_field", true)
                               .addField("bool_false_field", false)
                               .addField("uint_field", std::numeric_limits<unsigned int>::max())
                               .addField("ulonglong_field", std::numeric_limits<unsigned long long int>::max());

        // Set float precision to default for this test to ensure the expected double field value
        Point::floatsPrecision = defaultFloatsPrecision;
        CHECK_THAT(point.getFields(), Equals("int_field=3i,"
                                             "longlong_field=1234i,"
                                             "string_field=\"string value\","
                                             "double_field=3.858999999999999986,"
                                             "bool_true_field=true,"
                                             "bool_false_field=false,"
                                             "uint_field=4294967295u,"
                                             "ulonglong_field=18446744073709551615u"));
    }

    TEST_CASE("Measurement of signed int type", "[PointTest]")
    {
        const auto point = Point{"test"}
                               .addField("int_f", static_cast<int>(-123))
                               .addField("int0_f", static_cast<int>(0))
                               .addField("longlongint_f", static_cast<long long int>(-1234567890LL));
        CHECK_THAT(point.getFields(), Equals(R"(int_f=-123i,int0_f=0i,longlongint_f=-1234567890i)"));
    }

    TEST_CASE("Measurement of unsigned int type", "[PointTest]")
    {
        const auto point = Point{"test"}
                               .addField("uint_f", static_cast<unsigned int>(321))
                               .addField("ulonglongint_f", static_cast<unsigned long long int>(1234567890LL));
        CHECK_THAT(point.getFields(), Equals(R"(uint_f=321u,ulonglongint_f=1234567890u)"));
    }

    TEST_CASE("Measurement of double type", "[PointTest]")
    {
        const auto point = Point{"test"}
                               .addField("double_f", double{-456.78934345});
        CHECK_THAT(point.getFields(), StartsWith(R"(double_f=-456.78)"));
    }

    TEST_CASE("Measurement of bool type", "[PointTest]")
    {
        const auto point = Point{"test"}
                               .addField("bool_f", true);
        CHECK_THAT(point.getFields(), Equals(R"(bool_f=true)"));
    }

    TEST_CASE("Measurement of string type", "[PointTest]")
    {
        const auto point = Point{"test"}
                               .addField("stdstring_f", std::string{"aAa bBB Ccc"})
                               .addField("cstr_f", "dD Ee");
        CHECK_THAT(point.getFields(), Equals(R"(stdstring_f="aAa bBB Ccc",cstr_f="dD Ee")"));
    }

    TEST_CASE("Field with empty name is not added", "[PointTest]")
    {
        const auto point = Point{"test"}.addField("", "not added");
        CHECK_THAT(point.getFields(), Equals(""));
    }

    TEST_CASE("Field with empty value is added", "[PointTest]")
    {
        const auto point = Point{"test"}.addField("added", "");
        CHECK_THAT(point.getFields(), Equals("added=\"\""));
    }

    TEST_CASE("Measurement with tag", "[PointTest]")
    {
        const auto point = Point{"test"}.addTag("tag_name", "tag_value");
        CHECK_THAT(point.getTags(), Equals("tag_name=tag_value"));
    }

    TEST_CASE("Measurement with multiple tags", "[PointTest]")
    {
        const auto point = Point{"test"}
                               .addTag("tag_0", "value_0")
                               .addTag("tag_1", "value_1")
                               .addTag("tag_2", "value_2");
        CHECK_THAT(point.getTags(), Equals("tag_0=value_0,tag_1=value_1,tag_2=value_2"));
    }

    TEST_CASE("Empty tag value is not added", "[PointTest]")
    {
        const auto point = Point{"test"}.addTag("tag", "");
        CHECK_THAT(point.getTags(), Equals(""));
    }

    TEST_CASE("Empty tag key is not added", "[PointTest]")
    {
        const auto point = Point{"test"}.addTag("", "value");
        CHECK_THAT(point.getTags(), Equals(""));
    }

    TEST_CASE("Measurement with specific time stamp", "[PointTest]")
    {
        const std::chrono::time_point<std::chrono::system_clock> timeStamp{std::chrono::milliseconds{1572830915}};
        const auto point = Point{"test"}.setTimestamp(timeStamp);
        CHECK(point.getTimestamp() == timeStamp);
    }

    TEST_CASE("Float field precision can be adjusted", "[PointTest]")
    {
        Point::floatsPrecision = 3;
        const auto pointPrecision3 = Point{"test"}.addField("float_field", 3.123456789);
        CHECK_THAT(pointPrecision3.getFields(), Equals("float_field=3.123"));

        Point::floatsPrecision = 1;
        const auto pointPrecision1 = Point{"test"}.addField("float_field", 50.123456789);
        CHECK_THAT(pointPrecision1.getFields(), Equals("float_field=50.1"));

        const auto secondPointPrecision1 = Point{"test"}.addField("float_field", 5.99);
        CHECK_THAT(secondPointPrecision1.getFields(), Equals("float_field=6.0"));
    }

    TEST_CASE("Float field precision with scientific values", "[PointTest]")
    {
        Point::floatsPrecision = 5;
        const auto point0 = Point{"test"}.addField("float_field", 123456789.0);
        CHECK_THAT(point0.getFields(), Equals("float_field=123456789.00000"));

        const auto point1 = Point{"test"}.addField("float_field", 1.23456789E+8);
        CHECK_THAT(point1.getFields(), Equals("float_field=123456789.00000"));

        const auto point2 = Point{"test"}.addField("float_field", 1.23456789E-3);
        CHECK_THAT(point2.getFields(), Equals("float_field=0.00123"));

        const auto point3 = Point{"test"}.addField("float_field", 1.23456789E-6);
        CHECK_THAT(point3.getFields(), Equals("float_field=0.00000"));
    }
}
