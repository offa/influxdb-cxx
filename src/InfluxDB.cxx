///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "InfluxDB.h"

#include <memory>
#include <string>

namespace influxdb 
{

InfluxDB::InfluxDB(std::unique_ptr<Transport> transport) :
  mTransport(std::move(transport))
{
}

void InfluxDB::enableBuffering(const std::size_t size)
{
  mBufferSize = size;
  mBuffering = true;
}

void InfluxDB::flushBuffer() {
  if (!mBuffering) {
    return;
  }
  for (auto&& point : mBuffer) {
    transmit(std::move(point));
  }
}

void InfluxDB::addGlobalTag(std::string_view key, std::string_view value)
{
  if (!mGlobalTags.empty()) mGlobalTags += ",";
  mGlobalTags += key;
  mGlobalTags += "=";
  mGlobalTags += value;
}

InfluxDB::~InfluxDB()
{
  if (mBuffering) {
    flushBuffer();
  }
}

void InfluxDB::transmit(std::string&& point)
{
  mTransport->send(std::move(point));
}

void InfluxDB::write(Point&& metric)
{
  if (mBuffering) {
    mBuffer.emplace_back(metric.toLineProtocol());
  } else {
    transmit(metric.toLineProtocol());
  }
}
} // namespace influxdb
