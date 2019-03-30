# influxdb-cxx

[![Build Status](https://travis-ci.com/awegrzyn/influxdb-cxx.svg?branch=master)](https://travis-ci.com/awegrzyn/influxdb-cxx)

InfluxDB C++ client library

## Quick start

### Basic usage

```cpp
/// Provide complete URI
auto influxdb = influxdb::InfluxDBFactory::Get("udp://127.0.0.1:8094");
influxdb->write(Point{"test"}
  .addField("value", 10)
  .addTag("host", "localhost")
);
```

### Installation

__Build requirements:__
 - g++ 6.0+
 - CMake 3.12+
 - cURL
 - boost 1.57+ (optional for UDP and Unix socket transports)

```bash
git clone https://github.com/awegrzyn/influxdb-cxx.git
cd influxdb-cxx; mkdir build; cd build
cmake ..
make -j
sudo make install
```

## Transports

| Name        | Dependency  | URI protocol | Sample URI                            |
| ----------- |:-----------:|:------------:| -------------------------------------:|
| HTTP        | cURL        | `http`       | `http://localhost:8086/write?db=<db>` |
| UDP         | boost       | `udp`        | `udp://localhost:8094`                |
| Unix socket | boost       | `unix`       | `unix:///tmp/telegraf.sock`           |
