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
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "InfluxDB.h"
#include "InfluxDBException.h"
#include "LineProtocol.h"
#include "BoostSupport.h"
#include <memory>
#include <string>

namespace influxdb
{
    namespace
    {
        /// Group the points into the largest possible line-protocol messages that can be sent using the transport.
        template <typename PointContainer>
        void TransmitBatch(std::unique_ptr<Transport>& transport, const std::string& globalTags, PointContainer&& points)
        {
            LineProtocol formatter{globalTags};
            std::string lineProtocol;
            bool appendNewLine{false};
            bool messageSizeExceeded{false};

            const auto maxMessageSize{transport->getMaxMessageSize()};
            for (const auto& point : points)
            {
                auto formattedPoint{formatter.format(point)};
                auto GetRequiredSize{[&appendNewLine](const std::string& fp) -> std::size_t
                                     {
                                         // Have to recalculate because the point may fit in a new message if
                                         // it doesn't have a preceding newline.
                                         return appendNewLine ? 1 + fp.size() : fp.size();
                                     }};

                while (maxMessageSize < lineProtocol.size() + GetRequiredSize(formattedPoint))
                {
                    // Appending the current point would exceed the maximum message size.
                    // Flush the existing points and try again.
                    if (!lineProtocol.empty())
                    {
                        // If there is some existing content perhaps the current point will
                        // fit in a new message.
                        transport->send(std::move(lineProtocol));
                        lineProtocol.clear();
                        appendNewLine = false;
                    }
                    else
                    {
                        // Message is empty, so the current point is too large to be sent using this transport.
                        // Rather than throwing all the points away, we'll skip the current point and continue
                        // then raise an exception at the end.
                        messageSizeExceeded = true;
                        formattedPoint.clear();
                        break;
                    }
                }

                if (!formattedPoint.empty())
                {
                    if (appendNewLine)
                    {
                        lineProtocol += '\n';
                    }
                    lineProtocol += formattedPoint;
                    appendNewLine = true;
                }
            }

            // Send the last batch of points
            if (!lineProtocol.empty())
            {
                transport->send(std::move(lineProtocol));
            }

            // If any points were too large to be sent using this transport, throw an exception.
            if (messageSizeExceeded)
            {
                throw InfluxDBException{"One or more points exceeded the transport's maximum transmission size"};
            }
        }

        void TransmitPoint(std::unique_ptr<Transport>& transport, const std::string& globalTags, Point&& point)
        {
            LineProtocol formatter{globalTags};
            std::string formattedPoint{formatter.format(point)};
            if (formattedPoint.size() > transport->getMaxMessageSize())
            {
                throw InfluxDBException{"Point is too large to be sent using this transport"};
            }
            transport->send(std::move(formattedPoint));
        }
    }

    InfluxDB::InfluxDB(std::unique_ptr<Transport> transport)
        : mPointBatch{},
          mIsBatchingActivated{false},
          mBatchSize{0},
          mTransport(std::move(transport)),
          mGlobalTags{}
    {
        if (mTransport == nullptr)
        {
            throw InfluxDBException{"Transport must not be nullptr"};
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

    void InfluxDB::clearBatch()
    {
        mPointBatch.clear();
    }

    void InfluxDB::flushBatch()
    {
        if (mIsBatchingActivated && !mPointBatch.empty())
        {
            // Make sure that mPointBatch is cleared even if an exception is thrown during transmission.
            auto transmissionBatch{std::move(mPointBatch)};
            mPointBatch.clear();
            TransmitBatch(mTransport, mGlobalTags, std::move(transmissionBatch));
        }
    }


    void InfluxDB::addGlobalTag(std::string_view name, std::string_view value)
    {
        if (!mGlobalTags.empty())
        {
            mGlobalTags += ",";
        }
        mGlobalTags += LineProtocol::EscapeStringElement(LineProtocol::ElementType::TagKey, name);
        mGlobalTags += "=";
        mGlobalTags += LineProtocol::EscapeStringElement(LineProtocol::ElementType::TagValue, value);
    }

    void InfluxDB::write(Point&& point)
    {
        if (mIsBatchingActivated)
        {
            addPointToBatch(std::move(point));
        }
        else
        {
            TransmitPoint(mTransport, mGlobalTags, std::move(point));
        }
    }

    void InfluxDB::write(std::vector<Point>&& points)
    {
        if (mIsBatchingActivated)
        {
            for (auto&& point : points)
            {
                addPointToBatch(std::move(point));
            }
        }
        else
        {
            TransmitBatch(mTransport, mGlobalTags, points);
        }
    }

    std::string InfluxDB::execute(const std::string& cmd)
    {
        return mTransport->execute(cmd);
    }

    void InfluxDB::addPointToBatch(Point&& point)
    {
        mPointBatch.emplace_back(std::move(point));

        if (mPointBatch.size() >= mBatchSize)
        {
            flushBatch();
        }
    }

    std::vector<Point> InfluxDB::query(const std::string& query)
    {
        return internal::queryImpl(mTransport.get(), query);
    }

    void InfluxDB::createDatabaseIfNotExists()
    {
        mTransport->createDatabase();
    }

} // namespace influxdb
