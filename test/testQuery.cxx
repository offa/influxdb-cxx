#define BOOST_TEST_MODULE Test InfluxDB Query
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../include/InfluxDBFactory.h"

namespace influxdb {
namespace test {



BOOST_AUTO_TEST_CASE(query1)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
  auto points = influxdb->query("SELECT * from test WHERE host = 'localhost' LIMIT 3");
  BOOST_CHECK_EQUAL(points.size(), 3);
  BOOST_CHECK_EQUAL(points[0].getName(), "test");
  BOOST_CHECK_EQUAL(points[1].getName(), "test");
  BOOST_CHECK_EQUAL(points[2].getName(), "test");
  BOOST_CHECK_EQUAL(points[0].getFields(), "value=10");
  BOOST_CHECK_EQUAL(points[1].getFields(), "value=20");
  BOOST_CHECK_EQUAL(points[2].getFields(), "value=200");
}

BOOST_AUTO_TEST_CASE(failedQuery1)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
  auto points = influxdb->query("SELECT * from test1 WHERE host = 'localhost' LIMIT 3");
  BOOST_CHECK_EQUAL(points.size(), 0);
}

} // namespace test
} // namespace influxdb
