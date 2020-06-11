#define BOOST_TEST_MODULE Test InfluxDB
#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>
#include <random>

#include "HTTP.h"
#include "InfluxDBFactory.h"
#include "InfluxDBException.h"

namespace influxdb::test
{


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


BOOST_AUTO_TEST_CASE(pointWithEmptyStringFieldValueAreProperlyInserted)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");

  BOOST_CHECK_NO_THROW (
    influxdb->write(Point{"empty_string_field"}
                      .addField("str_value", ""))
  );
}

BOOST_AUTO_TEST_CASE(pointWithEmptyTagValueAreProperlyInserted)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");

  BOOST_CHECK_NO_THROW (
    influxdb->write(Point{"empty_string_tag"}
                      .addField("str_value", "")
                      .addTag("tag",""))
  );
}

BOOST_AUTO_TEST_CASE(pointsCanBeWrittenOneByOne)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");

  BOOST_CHECK_NO_THROW (
    influxdb->write(Point{"points_one_by_one"}
                      .addField("value", 10)
                      .addTag("host", "localhost"))
  );

  BOOST_CHECK_NO_THROW (
    influxdb->write(Point{"points_one_by_one"}
                      .addField("value", 20)
                      .addTag("host", "localhost"))
  );

  BOOST_CHECK_NO_THROW (
    influxdb->write(Point{"points_one_by_one"}
                      .addField("value", 200LL)
                      .addTag("host", "localhost"))
  );

  BOOST_CHECK_NO_THROW (
    influxdb->write(Point{"points_one_by_one"}
                      .addField("str_value", "lorem ipsum")
                      .addTag("host", "localhost"))
  );
}

BOOST_AUTO_TEST_CASE(pointsCanBeWrittenInVectorAsOneSingleBatch)
{
  std::vector<Point> points = {
    Point{"points_in_vector"}.addField("value", 10).addTag("host", "localhost"),
    Point{"points_in_vector"}.addField("value", 10).addTag("host", "localhost"),
    Point{"points_in_vector"}.addField("value", 200LL).addTag("host", "localhost"),
    Point{"points_in_vector"}.addField("str_value", "lorem ipsum").addTag("host", "localhost")
  };

  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");

  BOOST_CHECK_NO_THROW (
    influxdb->write(std::move(points))
  );
}

}
