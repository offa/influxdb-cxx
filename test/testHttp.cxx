#define BOOST_TEST_MODULE Test InfluxDB HTTP
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <random>

#include "HTTP.h"
#include "InfluxDBFactory.h"
#include "InfluxDBException.h"

namespace influxdb::test {

int getRandomNumber()
{
  std::vector<int> v;
  for (int i=0; i<1000; i++)
  {
    v.push_back(i);
  }
  std::random_device rd;
  std::mt19937 g(rd());

  std::shuffle(v.begin(), v.end(), g);
  return v[1];
}

BOOST_AUTO_TEST_CASE(write1)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
  influxdb->write(Point{"test"}
    .addField("value", 10)
    .addTag("host", "localhost"));

  influxdb->write(Point{"test"}
    .addField("value", 20)
    .addTag("host", "localhost"));

  influxdb->write(Point{"test"}
    .addField("value", 200LL)
    .addTag("host", "localhost"));

    influxdb->write(Point{"string"}
    .addField("value", "influxdb-cxx")
    .addTag("host", "localhost"));
}

BOOST_AUTO_TEST_CASE(writeWrongHost)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost2:8086/?db=test");
  BOOST_CHECK_THROW(influxdb->write(Point{"test"}.addField("value", 10)), InfluxDBException);
}

BOOST_AUTO_TEST_CASE(obtainDatabaseNameFromUrl)
{
  influxdb::transports::HTTP httpTransport("http://localhost:8086?db=test");
  BOOST_TEST("test" == httpTransport.databaseName());
}

BOOST_AUTO_TEST_CASE(obtainInfluxDbServiceFromUrl)
{
  influxdb::transports::HTTP httpTransport("http://localhost:8086/?db=test");
  BOOST_TEST("http://localhost:8086" == httpTransport.influxDbServiceUrl());
}

BOOST_AUTO_TEST_CASE(createsDatabaseAndSendPointProperly)
{
  std::stringstream ss;
  ss << "http://localhost:8086?db=new_" << getRandomNumber();
  influxdb::transports::HTTP httpTransport(ss.str());

  httpTransport.createDatabase();

  BOOST_CHECK_NO_THROW( httpTransport.send("test_measurement,tag1=one,tag2=two value1=0.64") );
}

BOOST_AUTO_TEST_CASE(sendingToNonexistentInfluxServerThrowsConectionError)
{
  influxdb::transports::HTTP httpTransport("http://unexistentServer:12345/?db=nonexistent_db");
  BOOST_CHECK_THROW( httpTransport.send("test_measurement,tag1=one,tag2=two value1=0.64"), ConnectionError );
}


BOOST_AUTO_TEST_CASE(sendingToNonExistentDatabaseThrowsNonExistentDatabase)
{
  influxdb::transports::HTTP httpTransport("http://localhost:8086?db=nonexistent_db");
  BOOST_CHECK_THROW( httpTransport.send("test_measurement,tag1=one,tag2=two value1=0.64"), NonExistentDatabase );
}


BOOST_AUTO_TEST_CASE(sendingAnIllFormedLineProtocolThrowsBadRequest)
{
  influxdb::transports::HTTP httpTransport("http://localhost:8086/?db=test");
  httpTransport.createDatabase();

  BOOST_CHECK_THROW( httpTransport.send("ill-formed line protocol"), BadRequest );
}

} // namespace influxdb::test
