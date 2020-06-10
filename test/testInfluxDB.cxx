#define BOOST_TEST_MODULE Test InfluxDB
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <random>

#include "HTTP.h"
#include "InfluxDBFactory.h"
#include "InfluxDBException.h"

namespace influxdb::test {


BOOST_AUTO_TEST_CASE(httpServiceCanCreateDatabase)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
  BOOST_CHECK_NO_THROW(influxdb->createDatabaseIfNotExists());
}

BOOST_AUTO_TEST_CASE(udpServiceCanNotCreateDatabase)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("udp://localhost:8086?db=test");
  BOOST_CHECK_THROW(influxdb->createDatabaseIfNotExists(), InfluxDBException);
}

BOOST_AUTO_TEST_CASE(unixServiceCanNotCreateDatabase)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("unix://localhost:8086?db=test");
  BOOST_CHECK_THROW(influxdb->createDatabaseIfNotExists(), InfluxDBException);
}

}
