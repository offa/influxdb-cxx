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

///
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "InfluxDB.h"
#include "InfluxDBException.h"
#include "LineProtocol.h"
#include "BoostSupport.h"
#include <iostream>
#include <memory>
#include <string>

namespace influxdb
{

InfluxDB::InfluxDB(std::unique_ptr<Transport> transport) :
  mPointBatch{},
  mIsBatchingActivated{false},
  mBatchSize{0},
  mTransport(std::move(transport)),
  mGlobalTags{}
{
  if (mTransport == nullptr)
  {
    throw InfluxDBException{"[InfluxDB]", "Transport must not be nullptr"};
  }
}

void InfluxDB::batchOf(std::size_t size)
{
  mBatchSize = size;
  mIsBatchingActivated = true;
}

std::size_t InfluxDB::batchSize() const
{
  return mPointBatch.size();
}

void InfluxDB::flushBatch()
{
  if (mIsBatchingActivated && !mPointBatch.empty())
  {
    transmit(joinLineProtocolBatch());
    mPointBatch.clear();
  }
}

std::string InfluxDB::joinLineProtocolBatch() const
{
  std::string joinedBatch;

  LineProtocol formatter{mGlobalTags};
  for (const auto &point : mPointBatch)
  {
    joinedBatch += formatter.format(point) + "\n";
  }

  joinedBatch.erase(std::prev(joinedBatch.end()));
  return joinedBatch;
}


void InfluxDB::addGlobalTag(std::string_view name, std::string_view value)
{
  if (!mGlobalTags.empty())
  {
      mGlobalTags += ",";
  }
  mGlobalTags += name;
  mGlobalTags += "=";
  mGlobalTags += value;
}

void InfluxDB::transmit(std::string &&point)
{
  mTransport->send(std::move(point));
}

void InfluxDB::write(Point &&point)
{
  if (mIsBatchingActivated)
  {
    addPointToBatch(std::move(point));
  }
  else
  {
    LineProtocol formatter{mGlobalTags};
    transmit(formatter.format(point));
  }
}

void InfluxDB::write(std::vector<Point> &&points)
{
  if (mIsBatchingActivated)
  {
    for (auto &&point : points)
    {
      addPointToBatch(std::move(point));
    }
  }
  else
  {
    std::string lineProtocol;
    LineProtocol formatter{mGlobalTags};

    for (const auto &point : points)
    {
      lineProtocol += formatter.format(point) + "\n";
    }

    lineProtocol.erase(std::prev(lineProtocol.end()));
    transmit(std::move(lineProtocol));
  }
}

void InfluxDB::addPointToBatch(Point &&point)
{
  mPointBatch.emplace_back(std::move(point));

  if (mPointBatch.size() >= mBatchSize)
  {
    flushBatch();
  }
}

std::vector<Point> InfluxDB::query(const std::string &query)
{
    return internal::queryImpl(mTransport.get(), query);
}

void InfluxDB::createDatabaseIfNotExists()
{
  try
  {
    mTransport->createDatabase();
  }
  catch (const std::runtime_error &)
  {
    throw InfluxDBException(__func__, "Transport did not allow create database");
  }
}

} // namespace influxdb
