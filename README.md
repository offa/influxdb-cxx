# influxdb-cxx

[![CI](https://github.com/offa/influxdb-cxx/workflows/ci/badge.svg)](https://github.com/offa/influxdb-cxx/actions)
[![GitHub release](https://img.shields.io/github/release/offa/influxdb-cxx.svg)](https://github.com/offa/influxdb-cxx/releases)
[![License](https://img.shields.io/badge/license-MIT-yellow.svg)](LICENSE)
![C++](https://img.shields.io/badge/c++-17-green.svg)


InfluxDB C++ client library
 - Batch write
 - Data exploration
 - Supported transports
   - HTTP/HTTPS with Basic Auth
   - UDP
   - Unix datagram socket


## Installation

 __Build requirements__
 - CMake 3.12+
 - C++17 compiler

__Dependencies__
 - [cpr](https://github.com/libcpr/cpr) (required)
 - boost 1.66+ (optional – see [Transports](#transports))

### Generic
 ```bash
mkdir build && cd build
cmake ..
sudo make install
 ```

## Quick start

### Include in CMake project

The InfluxDB library is exported as target `InfluxData::InfluxDB`.

```cmake
project(example)

find_package(InfluxDB)

add_executable(example-influx main.cpp)
target_link_libraries(example-influx PRIVATE InfluxData::InfluxDB)
```

This target is also provided when the project is included as a subdirectory.

```cmake
project(example)
add_subdirectory(influxdb-cxx)
add_executable(example-influx main.cpp)
target_link_libraries(example-influx PRIVATE InfluxData::InfluxDB)
```

### Basic write

```cpp
// Provide complete URI
auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
influxdb->write(influxdb::Point{"test"}
  .addField("value", 10)
  .addTag("host", "localhost")
);
```

### Batch write

```cpp
auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
// Write batches of 100 points
influxdb->batchOf(100);

for (;;) {
  influxdb->write(influxdb::Point{"test"}.addField("value", 10));
}
```

###### Note:

When batch write is enabled, call `flushBatch()` to flush pending batches.
This is of particular importance to ensure all points are written prior to destruction.

```cpp
auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
influxdb->batchOf(3);
influxdb->write(influxdb::Point{"test"}.addField("value", 1));
influxdb->write(influxdb::Point{"test"}.addField("value", 2));

// Flush batches, both points are written
influxdb->flushBatch();
```


### Query

```cpp
// Available over HTTP only
auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");
/// Pass an IFQL to get list of points
std::vector<influxdb::Point> points = influxdb->query("SELECT * FROM test");
```

### Execute cmd

```cpp
auto influxdb = influxdb::InfluxDBFactory::Get("http://localhost:8086?db=test");

// Execute a command and receive it's response
const auto response = influxdb->execute("SHOW DATABASES");
```

## Transports

An underlying transport is fully configurable by passing an URI:
```
[protocol]://[username:password@]host:port[?db=database]
```
<br>
List of supported transport is following:

| Name        | Dependency  | URI protocol   | Sample URI                            |
| ----------- |:-----------:|:--------------:| -------------------------------------:|
| HTTP        | cpr<sup>i)</sup> | `http`/`https` | `http://localhost:8086?db=<db>`  |
| TCP         | boost       | `tcp`          | `tcp://localhost:8094`                |
| UDP         | boost       | `udp`          | `udp://localhost:8094`                |
| Unix socket | boost       | `unix`         | `unix:///tmp/telegraf.sock`           |


<sup>i)</sup> boost is needed to support queries.


## InfluxDB v2.x compatibility

The support for InfluxDB v2.x is limited at the moment. It's possible to use the v1.x compatibility backend though.

Please visit [*InfluxDB 1.x compatibility API* docs](https://docs.influxdata.com/influxdb/v2.6/reference/api/influxdb-1x/) for more information.

To create a v1.x compatible user (as described [here](https://docs.influxdata.com/influxdb/v2.6/reference/cli/influx/v1/auth/)):

```sh
influx v1 auth create --read-bucket ${BUCKET_ID} --write-bucket ${BUCKET_ID} --username ${USERNAME} --password ${PASSWORD}
```
