///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#ifndef INFLUXDATA_EXCEPTION_H
#define INFLUXDATA_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace influxdb {

class InfluxDBException : public std::runtime_error {
public:
  InfluxDBException(const std::string &source, const std::string &message) : std::runtime_error::runtime_error(
          "influx-cxx [" + source + "]: " + message) {}
};

class NonExistentDatabase : public InfluxDBException {
public:
  NonExistentDatabase(const std::string &source, const std::string &message) : InfluxDBException(source, message) {}
};

class BadRequest : public InfluxDBException {
public:
  BadRequest(const std::string &source, const std::string &message) : InfluxDBException(source, message) {}
};

class ServerError : public InfluxDBException {
public:
  ServerError(const std::string &source, const std::string &message) : InfluxDBException(source, message) {}
};

class ConnectionError : public InfluxDBException {
public:
  ConnectionError(const std::string &source, const std::string &message) : InfluxDBException(source, message) {};
};


} // namespace influxdb

#endif // INFLUXDATA_EXCEPTION_H
