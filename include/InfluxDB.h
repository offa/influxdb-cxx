// MIT License
//
// Copyright (c) 2020-2023 offa
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
#include "influxdb_export.h"

namespace influxdb
{

    class INFLUXDB_EXPORT InfluxDB
    {
    public:
        /// Disable copy constructor
        InfluxDB& operator=(const InfluxDB&) = delete;

        /// Disable copy constructor
        InfluxDB(const InfluxDB&) = delete;

        /// Constructor required valid transport
        explicit InfluxDB(std::unique_ptr<Transport> transport);

        /// Writes a point
        /// \param point
        void write(Point&& point);

        /// Writes a vector of point
        /// \param point
        void write(std::vector<Point>&& points);

        /// Queries InfluxDB database
        std::vector<Point> query(const std::string& query);

        /// Create InfluxDB database if does not exists
        void createDatabaseIfNotExists();

        /// Flushes points batched (this can also happens when buffer is full)
        void flushBatch();

        /// \deprecated use \ref flushBatch() instead - will be removed in v0.8.0
        [[deprecated("Use flushBatch() instead - will be removed in v0.8.0")]] inline void flushBuffer()
        {
            flushBatch();
        }

        /// Enables points batching
        /// \param size
        void batchOf(std::size_t size = 32);

        /// Returns current batch size
        std::size_t batchSize() const;

        /// Clears the point batch
        void clearBatch();

        /// Adds a global tag
        /// \param name
        /// \param value
        void addGlobalTag(std::string_view name, std::string_view value);

        /// Executes a command and returns it's response.
        /// \param cmd
        std::string execute(const std::string& cmd);

    private:
        void addPointToBatch(Point&& point);

        /// line protocol batch to be written
        std::deque<Point> mPointBatch;

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
