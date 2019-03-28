# influxdb-cxx
InfluxDB C++ client library

## Installation

1. Requirements: boost::asio and cURL
2. `mkdir build; cd build`
3. `cmake ..; make -j`

## Sample usage
```cpp
auto influxdb = influxdb::InfluxDBFactory::Get("udp://127.0.0.1:1234");
influxdb->write(Point{"test"}
  .addField("value", 10) 
  .addTag("host", "localhost")
);
```
