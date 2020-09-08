#define BOOST_TEST_MODULE Test InfluxDB Point
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "InfluxDBFactory.h"
#include <iterator>

namespace influxdb::test {

std::vector<std::string> getVector(const Point& point)
{
  std::istringstream result(point.toLineProtocol());
  return std::vector<std::string>{std::istream_iterator<std::string>{result},
                      std::istream_iterator<std::string>{}};
}

BOOST_AUTO_TEST_CASE(test1)
{
  auto point = Point{"test"}
    .addField("value", 10LL);

  auto result = getVector(point);

  BOOST_CHECK_EQUAL(result[0], "test");
  BOOST_CHECK_EQUAL(result[1], "value=10i");
}

BOOST_AUTO_TEST_CASE(test2)
{
  Point::floatsPrecision = 1;
  auto point = Point{"test"}
    .addField("value", 10LL)
    .addField("dvalue", 10.1);

  auto result = getVector(point);

  BOOST_CHECK_EQUAL(result[0], "test");
  BOOST_CHECK_EQUAL(result[1], "value=10i,dvalue=10.1");
}

BOOST_AUTO_TEST_CASE(test3)
{
  Point::floatsPrecision = 1;
  auto point = Point{"test"}
    .addField("value", 10LL)
    .addField("dvalue", 10.1)
    .addTag("tag", "tagval");

  auto result = getVector(point);

  BOOST_CHECK_EQUAL(result[0], "test,tag=tagval");
  BOOST_CHECK_EQUAL(result[1], "value=10i,dvalue=10.1");
}

BOOST_AUTO_TEST_CASE(test4)
{
  auto point = Point{"test"}
    .addField("value", 10)
    .addField("value", 100LL)
    .setTimestamp(std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds(1572830914)));

  auto result = getVector(point);
  BOOST_CHECK_EQUAL(result[2], "1572830914000000");
}

BOOST_AUTO_TEST_CASE(fieldsWithEmptyNameAreNotAdded)
{
  auto point = Point{"test"}.addField("", 10);

  BOOST_CHECK_EQUAL(point.getFields().empty(), true);
}

BOOST_AUTO_TEST_CASE(floatFieldsPrecisionCanBeAdjusted)
{
  Point::floatsPrecision = 3;
  auto pointPrecision3 = Point{"test"}.addField("float_field", 3.123456789);
  BOOST_CHECK_EQUAL(pointPrecision3.getFields(), "float_field=3.123");

  Point::floatsPrecision = 1;
  auto pointPrecision1 = Point{"test"}.addField("float_field", 50.123456789);
  BOOST_CHECK_EQUAL(pointPrecision1.getFields(), "float_field=50.1");

  pointPrecision1 = Point{"test"}.addField("float_field", 5.99);
  BOOST_CHECK_EQUAL(pointPrecision1.getFields(), "float_field=6.0");
}

BOOST_AUTO_TEST_CASE(floatFieldsPrecisionWithScientificValues)
{
  Point::floatsPrecision = 5;
  auto pointPrecision5 = Point{"test"}.addField("float_field", 123456789.0);
  BOOST_CHECK_EQUAL(pointPrecision5.getFields(), "float_field=123456789.00000");

  pointPrecision5 = Point{"test"}.addField("float_field", 1.23456789E+8);
  BOOST_CHECK_EQUAL(pointPrecision5.getFields(), "float_field=123456789.00000");

  pointPrecision5 = Point{"test"}.addField("float_field", 1.23456789E-3);
  BOOST_CHECK_EQUAL(pointPrecision5.getFields(), "float_field=0.00123");

  pointPrecision5 = Point{"test"}.addField("float_field", 1.23456789E-6);
  BOOST_CHECK_EQUAL(pointPrecision5.getFields(), "float_field=0.00000");
}

} // namespace influxdb::test
