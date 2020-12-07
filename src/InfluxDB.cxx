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

#include <iostream>
#include <memory>
#include <string>

#ifdef INFLUXDB_WITH_BOOST

#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#endif

namespace influxdb
{

InfluxDB::InfluxDB(std::unique_ptr<Transport> transport) :
  mLineProtocolBatch{},
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

void InfluxDB::batchOf(const std::size_t size)
{
  mBatchSize = size;
  mIsBatchingActivated = true;
}

void InfluxDB::flushBatch()
{
  if (mIsBatchingActivated && !mLineProtocolBatch.empty())
  {
    transmit(joinLineProtocolBatch());
    mLineProtocolBatch.clear();
  }
}

std::string InfluxDB::joinLineProtocolBatch() const
{
  std::string joinedBatch;
  for (const auto &line : mLineProtocolBatch)
  {
    joinedBatch += line + "\n";
  }

  joinedBatch.erase(std::prev(joinedBatch.cend()));
  return joinedBatch;
}


void InfluxDB::addGlobalTag(std::string_view key, std::string_view value)
{
  if (!mGlobalTags.empty())
  {
      mGlobalTags += ",";
  }
  mGlobalTags += key;
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
    addPointToBatch(point);
  }
  else
  {
    transmit(point.toLineProtocol());
  }
}

void InfluxDB::write(std::vector<Point> &&points)
{
  if (mIsBatchingActivated)
  {
    for (const auto &point : points)
    {
      addPointToBatch(point);
    }
  }
  else
  {
    std::string lineProtocol;
    for (const auto &point : points)
    {
      lineProtocol += point.toLineProtocol() + "\n";
    }

    lineProtocol.erase(std::prev(lineProtocol.cend()));
    transmit(std::move(lineProtocol));
  }
}

void InfluxDB::addPointToBatch(const Point &point)
{
  mLineProtocolBatch.emplace_back(point.toLineProtocol());
  if (mLineProtocolBatch.size() >= mBatchSize)
  {
    flushBatch();
  }
}

#ifdef INFLUXDB_WITH_BOOST

std::vector<Point> InfluxDB::query(const std::string &query)
{
  const auto response = mTransport->query(query);
  std::stringstream responseString;
  responseString << response;
  std::vector<Point> points;
  boost::property_tree::ptree pt;
  boost::property_tree::read_json(responseString, pt);

  for (const auto &result : pt.get_child("results"))
  {
    const auto isResultEmpty = result.second.find("series");
    if (isResultEmpty == result.second.not_found())
    {
        return {};
    }
    for (const auto &series : result.second.get_child("series"))
    {
      const auto columns = series.second.get_child("columns");

      for (const auto &values : series.second.get_child("values"))
      {
        Point point{series.second.get<std::string>("name")};
        auto iColumns = columns.begin();
        auto iValues = values.second.begin();
        for (; iColumns != columns.end() && iValues != values.second.end(); ++iColumns, ++iValues)
        {
          const auto value = iValues->second.get_value<std::string>();
          const auto column = iColumns->second.get_value<std::string>();
          if (!column.compare("time"))
          {
            std::tm tm = {};
            std::stringstream timeString;
            timeString << value;
            timeString >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
            point.setTimestamp(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
            continue;
          }
          // cast all values to double, if strings add to tags
          try
          {
              point.addField(column, boost::lexical_cast<double>(value));
          }
          catch (...)
          {
              point.addTag(column, value);
          }
        }
        points.push_back(std::move(point));
      }
    }
  }
  return points;
}

#else
std::vector<Point> InfluxDB::query([[maybe_unused]] const std::string& query)
{
  throw InfluxDBException("InfluxDB::query", "Boost is required");
}
#endif

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
