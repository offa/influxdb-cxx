///
/// \author Adam Wegrzynek
///

#ifndef INFLUXDATA_INFLUXDB_H
#define INFLUXDATA_INFLUXDB_H

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <deque>

#include "Transport.h"
#include "Point.h"

namespace influxdb
{

class InfluxDB
{
  public:
    /// Disable copy constructor
    InfluxDB & operator=(const InfluxDB&) = delete;

    /// Disable copy constructor
    InfluxDB(const InfluxDB&) = delete;

    /// Constructor required valid transport
    InfluxDB(std::unique_ptr<Transport> transport);

    /// Flushes buffer
    ~InfluxDB();

    /// Writes a point
    /// \param point
    void write(Point&& point);

    /// Writes a vector of point
    /// \param point
    void write(std::vector<Point> &&points);

    /// Queries InfluxDB database
    std::vector<Point> query(const std::string& query);

    /// Create InfluxDB database if does not exists
    void createDatabaseIfNotExists();

    /// Flushes points batched (this can also happens when buffer is full)
    void flushBatch();

    /// Enables points batching
    /// \param size
    void batchOf(const std::size_t size = 32);

    /// Adds a global tag
    /// \param name
    /// \param value
    void addGlobalTag(std::string_view name, std::string_view value);

private:
    void addPointToBatch(const Point &point);

  private:
    /// line protocol batch to be writen
    std::deque<std::string> mLineProtocolBatch;

    /// Flag stating whether point buffering is enabled
    bool mIsBatchingActivated;

    /// Points batch size
    std::size_t mBatchSize;

    /// Underlying transport UDP/HTTP/Unix socket
    std::unique_ptr<Transport> mTransport;

    /// Transmits string over transport
    void transmit(std::string&& point);

    /// List of global tags
    std::string mGlobalTags;


  std::string joinLineProtocolBatch() const;
};

} // namespace influxdb

#endif // INFLUXDATA_INFLUXDB_H
