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

#define BOOST_TEST_MODULE Test InfluxDB UDP
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "InfluxDBFactory.h"

namespace influxdb::test {

BOOST_AUTO_TEST_CASE(test)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("udp://localhost:8084");
  influxdb->write(Point{"test"}
    .addField("value", 10)
    .addField("value", 20)
    .addField("value", 100LL)
    .addTag("host", "adampc")
  );
}

BOOST_AUTO_TEST_CASE(test2)
{
  auto influxdb = influxdb::InfluxDBFactory::Get("udp://localhost:8084");
  influxdb->batchOf(2);
  influxdb->write(Point{"test"}.addField("value", 10));
  influxdb->write(Point{"test"}.addField("value", 10));
  influxdb->write(Point{"test"}.addField("value", 10));
  influxdb->write(Point{"test"}.addField("value", 100LL));
  influxdb->write(Point{"test"}.addField("value", 100LL));
  influxdb->write(Point{"test"}.addField("value", 100LL));
}

} // namespace influxdb::test
