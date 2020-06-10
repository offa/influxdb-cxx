#define BOOST_TEST_MODULE Test InfluxDB Factory
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "InfluxDBFactory.h"
#include "InfluxDBException.h"

namespace influxdb::test {


BOOST_AUTO_TEST_CASE(unrecognisedBackend)
{
  BOOST_CHECK_THROW(influxdb::InfluxDBFactory::Get("httpz://localhost:8086?db=test"), InfluxDBException);
}

BOOST_AUTO_TEST_CASE(missformatedUrl)
{
  BOOST_CHECK_THROW(influxdb::InfluxDBFactory::Get("localhost:8086?db=test"), InfluxDBException);
}

BOOST_AUTO_TEST_CASE(missingDb)
{
  BOOST_CHECK_THROW(influxdb::InfluxDBFactory::Get("http://localhost:8086"), InfluxDBException);
}

} // namespace influxdb::test
