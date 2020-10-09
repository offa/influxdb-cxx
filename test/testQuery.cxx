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

#define BOOST_TEST_MODULE Test InfluxDB Query
#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>
#include "InfluxDBFactory.h"
#include "InfluxDBException.h"

namespace influxdb::test {



BOOST_AUTO_TEST_CASE(query1)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=testQuery");
  influxdb->createDatabaseIfNotExists();

  std::vector<Point> pointsWritten = {
    Point{"query1"}.addField("value", 10).addTag("host", "localhost"),
    Point{"query1"}.addField("value", 20).addTag("host", "localhost"),
    Point{"query1"}.addField("value", 200LL).addTag("host", "localhost")
  };

  influxdb->write(std::move(pointsWritten));

  auto points = influxdb->query("SELECT * from query1 WHERE host = 'localhost' ORDER BY time DESC LIMIT 3");

  // unfortunately no type checking is done at query. It converts every value in double
  // so all point fields are treated as double fields. It makes necessary to adapt Point precision of float fields
  // or consider default number of decimal digits at the checking
  BOOST_CHECK_EQUAL(points.size(), 3);
  BOOST_CHECK_EQUAL(points[0].getName(), "query1");
  BOOST_CHECK_EQUAL(points[1].getName(), "query1");
  BOOST_CHECK_EQUAL(points[2].getName(), "query1");
  BOOST_CHECK_EQUAL(points[0].getFields(), "value=200.000000000000000000");
  BOOST_CHECK_EQUAL(points[1].getFields(), "value=20.000000000000000000");
  BOOST_CHECK_EQUAL(points[2].getFields(), "value=10.000000000000000000");
  BOOST_CHECK_EQUAL(points[0].getTags(), "host=localhost");
  BOOST_CHECK_EQUAL(points[1].getTags(), "host=localhost");
  BOOST_CHECK_EQUAL(points[2].getTags(), "host=localhost");
}

BOOST_AUTO_TEST_CASE(timeStampVerify)
{
  double timeZone = 3600; //+1h

  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=testQuery");
  influxdb->createDatabaseIfNotExists();

  Point point = Point{"timeStampVerify"}.addField("value", 10);
  auto timestamp = point.getTimestamp();
  influxdb->write(std::move(point));

  auto points = influxdb->query("SELECT * from timeStampVerify ORDER BY DESC LIMIT 1");
  std::chrono::duration<double> diff = timestamp - points[0].getTimestamp();
  double diffZone = diff.count() - timeZone;
  BOOST_CHECK(diffZone < 1); // 1s
}

BOOST_AUTO_TEST_CASE(failedQuery1)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
  auto points = influxdb->query("SELECT * from test1 WHERE host = 'localhost' LIMIT 3");
  BOOST_CHECK_EQUAL(points.size(), 0);
}

BOOST_AUTO_TEST_CASE(failedQuery2)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
  BOOST_CHECK_THROW(influxdb->query("SELECT *from test1 WHEREhost = 'localhost' LIMIT 3"), InfluxDBException);
}

} // namespace influxdb::test
