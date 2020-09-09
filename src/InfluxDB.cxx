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
}

InfluxDB::~InfluxDB()
{
  if (!mIsBatchingActivated)
  {
    return;
  }
  flushBatch();
}

void InfluxDB::batchOf(const std::size_t size)
{
  mBatchSize = size;
  mIsBatchingActivated = true;
}

void InfluxDB::flushBatch()
{
  if (!mIsBatchingActivated || mLineProtocolBatch.empty())
  {
    return;
  }

  transmit(joinLineProtocolBatch());
}


std::string InfluxDB::joinLineProtocolBatch() const
{
  std::string joinedBatch;
  for (const auto &line : mLineProtocolBatch)
  {
    joinedBatch += line + "\n";
  }
  return joinedBatch;
}


void InfluxDB::addGlobalTag(std::string_view key, std::string_view value)
{
  if (!mGlobalTags.empty())
  { mGlobalTags += ","; }
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
  auto response = mTransport->query(query);
  std::stringstream ss;
  ss << response;
  std::vector<Point> points;
  boost::property_tree::ptree pt;
  boost::property_tree::read_json(ss, pt);

  for (auto &result : pt.get_child("results"))
  {
    auto isResultEmpty = result.second.find("series");
    if (isResultEmpty == result.second.not_found())
    { return {}; }
    for (auto &series : result.second.get_child("series"))
    {
      auto columns = series.second.get_child("columns");

      for (auto &values : series.second.get_child("values"))
      {
        Point point{series.second.get<std::string>("name")};
        auto iColumns = columns.begin();
        auto iValues = values.second.begin();
        for (; iColumns != columns.end() && iValues != values.second.end(); iColumns++, iValues++)
        {
          auto value = iValues->second.get_value<std::string>();
          auto column = iColumns->second.get_value<std::string>();
          if (!column.compare("time"))
          {
            std::tm tm = {};
            std::stringstream ss;
            ss << value;
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
            point.setTimestamp(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
            continue;
          }
          // cast all values to double, if strings add to tags
          try
          { point.addField(column, boost::lexical_cast<double>(value)); }
          catch (...)
          { point.addTag(column, value); }
        }
        points.push_back(std::move(point));
      }
    }
  }
  return points;
}

void InfluxDB::createDatabaseIfNotExists()
{
  try
  {
    mTransport->createDatabase();
  }
  catch (const std::runtime_error &error)
  {
    throw InfluxDBException(__func__, "Transport did not allow create database");
  }
}

#else
std::vector<Point> InfluxDB::query(const std::string& /*query*/)
{
  throw InfluxDBException("InfluxDB::query", "Boost is required");
}
#endif

} // namespace influxdb
