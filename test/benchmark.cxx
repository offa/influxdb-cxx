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

#include  <InfluxDBFactory.h>
#include <boost/program_options.hpp>
#include <iostream>

using namespace influxdb;

int main(int argc, char* argv[]) {

  int count = 1;

  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    ("count", boost::program_options::value<int>(), "Points count")
    ("url", boost::program_options::value<std::string>()->required(), "URL to InfluxDB database")
    ("buffer", boost::program_options::value<int>(), "Buffer size");

  boost::program_options::variables_map vm;
  boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  auto db = InfluxDBFactory::Get(vm["url"].as<std::string>());

  if (vm.count("count")) {
    count = std::floor(vm["count"].as<int>()/2) + 1;
  }

  if (vm.count("buffer")) {
    db->batchOf(vm["buffer"].as<int>());
  }

  for(int i = 0; i <= count; i++) {
    db->write(Point{"int"}.addField("value1", 10).addField("value2", "11").addTag("tag1", "machine"));
    db->write(Point{"double"}.addField("value1", 10.10).addField("value2", "11.11").addTag("tag1", "machine"));
  }
  std::cout << "Written " << 2*count-1 << " points";
  if (vm.count("buffer")) { std::cout << " through " << vm["buffer"].as<int>(); }
  std::cout << " to " << vm["url"].as<std::string>() << std::endl;
}
