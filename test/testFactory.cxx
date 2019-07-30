#define BOOST_TEST_MODULE Test InfluxDB Factory
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../include/InfluxDBFactory.h"

namespace influxdb {
namespace test {

BOOST_AUTO_TEST_CASE(test)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("udp://localhost:8084");
  influxdb->write(Point{"test"}
    .addField("value", 10)
    .addTag("host", "adampc")
  );
}

} // namespace test
} // namespace influxdb
