// MIT License
//
// Copyright (c) 2019 Adam Wegrzynek
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

#define BOOST_TEST_MODULE Test InfluxDB HTTP
#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>
#include <random>
#include <algorithm>
#include <numeric>

#include "HTTP.h"
#include "InfluxDBFactory.h"
#include "InfluxDBException.h"

/**
 * \deprecated related test has been moved
 */

namespace influxdb::test {

int getRandomNumber()
{
  std::vector<int> v(1000);
  std::iota(v.begin(), v.end(), 0);

  std::random_device rd;
  std::mt19937 g(rd());

  std::shuffle(v.begin(), v.end(), g);
  return v[1];
}

BOOST_AUTO_TEST_CASE(write1)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
  influxdb->createDatabaseIfNotExists();
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
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost2:8086?db=test");
  BOOST_CHECK_THROW(influxdb->write(Point{"test"}.addField("value", 10)), InfluxDBException);
}

BOOST_AUTO_TEST_CASE(obtainDatabaseNameFromUrl)
{
  influxdb::transports::HTTP httpTransport("http://localhost:8086?db=test");
  BOOST_TEST("test" == httpTransport.databaseName());
}

BOOST_AUTO_TEST_CASE(obtainInfluxDbServiceFromUrl)
{
  influxdb::transports::HTTP httpTransport("http://localhost:8086?db=test");
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
  influxdb::transports::HTTP httpTransport("http://unexistentServer:12345?db=nonexistent_db");
  BOOST_CHECK_THROW( httpTransport.send("test_measurement,tag1=one,tag2=two value1=0.64"), ConnectionError );
}

BOOST_AUTO_TEST_CASE(sendingToNonExistentDatabaseThrowsNonExistentDatabase)
{
  influxdb::transports::HTTP httpTransport("http://localhost:8086?db=nonexistent_db");
  BOOST_CHECK_THROW( httpTransport.send("test_measurement,tag1=one,tag2=two value1=0.64"), NonExistentDatabase );
}


BOOST_AUTO_TEST_CASE(sendingAnIllFormedLineProtocolThrowsBadRequest)
{
  influxdb::transports::HTTP httpTransport("http://localhost:8086?db=test");
  httpTransport.createDatabase();

  BOOST_CHECK_THROW( httpTransport.send("ill-formed line protocol"), BadRequest );
}

BOOST_AUTO_TEST_CASE(queryUrlFormat)
{
    auto queryUrl = [](const auto& url) {
        influxdb::transports::HTTP transport{url};
        transport.query("SELECT * FROM notexisting");
    };

    BOOST_CHECK_NO_THROW(queryUrl("http://localhost:8086?db=test"));
    BOOST_CHECK_NO_THROW(queryUrl("http://localhost:8086/?db=test"));
}

} // namespace influxdb::test
