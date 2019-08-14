# influxdb-cxx

[![Build Status](https://travis-ci.com/awegrzyn/influxdb-cxx.svg?branch=master)](https://travis-ci.com/awegrzyn/influxdb-cxx)
[![codecov](https://codecov.io/gh/awegrzyn/influxdb-cxx/branch/master/graph/badge.svg)](https://codecov.io/gh/awegrzyn/influxdb-cxx)


InfluxDB C++ client library
 - Writing points
 - Batch write
 - ~~Data exploration~~
 - Supported transports
   - HTTP/HTTPS with Basic Auth
   - UDP
   - Unix datagram socket


 ### Installation

 __Build requirements:__
 g++ 6.0+, CMake 3.12+

__Dependencies:__
 cURL, boost 1.57+ (optional)

 __Compilation__
 ```bash
git clone https://github.com/awegrzyn/influxdb-cxx.git
cd influxdb-cxx; mkdir build
cd build
cmake ..
sudo make install
 ```

## Quick start

### Basic usage

```cpp
// Provide complete URI
auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086/write?db=test");
influxdb->write(Point{"test"}
  .addField("value", 10)
  .addTag("host", "localhost")
);
```

### Batch write

```cpp
// Provide complete URI
auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086/write?db=test");
// Write batches of 100 points
influxdb->batchOf(100);

for (;;) {
  influxdb->write(Point{"test"}.addField("value", 10));
}
```

## Transports

An underlying transport is fully configurable by passing an URI:
```
[protocol]://[username:password@]host:port[/write?db=database]
```
<br>
List of supported transport is following:

| Name        | Dependency  | URI protocol | Sample URI                            |
| ----------- |:-----------:|:------------:| -------------------------------------:|
| HTTP        | cURL        | `http`/`https`       | `http://localhost:8086/write?db=<db>` |
| UDP         | boost       | `udp`        | `udp://localhost:8094`                |
| Unix socket | boost       | `unix`       | `unix:///tmp/telegraf.sock`           |
