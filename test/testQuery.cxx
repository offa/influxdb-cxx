#define BOOST_TEST_MODULE Test InfluxDB Query
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../include/InfluxDBFactory.h"

namespace influxdb {
namespace test {



BOOST_AUTO_TEST_CASE(query1)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
  auto points = influxdb->query("SELECT * from test LIMIT 2");
  BOOST_CHECK_EQUAL(points.front().getName(), "test");
  BOOST_CHECK_EQUAL(points.back().getName(), "test");
  BOOST_CHECK_EQUAL(points.front().getFields(), "value=10");
  BOOST_CHECK_EQUAL(points.back().getFields(), "value=20");
}

} // namespace test
} // namespace influxdb
