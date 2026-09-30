// MIT License
//
// Copyright (c) 2020-2026 offa
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

#include "BoostSupport.h"
#include "UDP.h"
#include "TCP.h"
#include "UnixSocket.h"
#include <chrono>
#include <format>
#include <iterator>
#include <charconv>
#include <nlohmann/json.hpp>
#include <date/date.h>

namespace influxdb::internal
{
    namespace
    {
        std::chrono::sys_time<std::chrono::nanoseconds> parseTimeStamp(const std::string& value)
        {
            std::istringstream timeString{value};
            std::chrono::sys_time<std::chrono::nanoseconds> timeStamp{};
            date::from_stream(timeString, "%FT%T%Z", timeStamp);

            return timeStamp;
        }

        std::string valueToString(const nlohmann::json& value)
        {
            if (value.is_string())
            {
                return value.get<std::string>();
            }
            if (value.is_number())
            {
                return std::to_string(value.get<double>());
            }

            if (value.is_null())
            {
                return {};
            }

            if (value.is_boolean())
            {
                return value.get<bool>() ? "true" : "false";
            }

            return value.dump();
        }

        void addTags(Point& point, const nlohmann::json& series)
        {
            const auto itr = series.find("tags");
            if (itr == series.end() || !itr->is_object())
            {
                return;
            }

            for (const auto& [name, value] : itr->items())
            {
                point.addTag(name, valueToString(value));
            }
        }

        void addValues(Point& point, const nlohmann::json& columns, const nlohmann::json& row)
        {
            const std::size_t count = std::min(columns.size(), row.size());

            for (std::size_t i = 0; i < count; ++i)
            {
                if (!columns[i].is_string())
                {
                    continue;
                }

                const std::string column = columns[i].get<std::string>();
                const nlohmann::json& value = row[i];

                if (column == "time")
                {
                    point.setTimestamp(parseTimeStamp(valueToString(value)));
                }
                else if (value.is_number())
                {
                    point.addField(column, value.get<double>());
                }
                else if (value.is_boolean())
                {
                    point.addField(column, value.get<bool>());
                }
                else if (value.is_string())
                {
                    std::string text = value.get<std::string>();
                    double number{};
                    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), number);

                    if (ec == std::errc{} && ptr == text.data() + text.size())
                    {
                        point.addField(column, number);
                    }
                    else
                    {
                        point.addField(column, std::move(text));
                    }
                }
                else
                {
                    point.addTag(column, valueToString(value));
                }
            }
        }

        std::vector<Point> processSeries(const nlohmann::json& series)
        {
            if (!series.is_object())
            {
                throw std::runtime_error("InfluxDB query error: 'series' element is not an object");
            }

            const auto columnsItr = series.find("columns");
            const auto valuesItr = series.find("values");

            if (columnsItr == series.end() || valuesItr == series.end() ||
                !columnsItr->is_array() || !valuesItr->is_array())
            {
                return {};
            }

            std::vector<Point> points;
            for (const auto& row : *valuesItr)
            {
                if (row.is_array())
                {
                    Point point{series.value("name", "")};
                    addTags(point, series);
                    addValues(point, *columnsItr, row);
                    points.push_back(std::move(point));
                }
            }
            return points;
        }

        void checkError(const nlohmann::json& obj)
        {
            if (const auto err = obj.find("error"); err != obj.end())
            {
                throw std::runtime_error(std::format("InfluxDB query error: {}", valueToString(*err)));
            }
        }

    }


    std::vector<Point> queryImpl(Transport* transport, const std::string& query)
    {
        try
        {
            const auto document = nlohmann::json::parse(transport->query(query));
            checkError(document);

            if (const auto errorItr = document.find("error"); errorItr != document.end())
            {
                throw std::runtime_error(std::format("InfluxDB query error: {}", valueToString(*errorItr)));
            }

            const auto resultsItr = document.find("results");

            if (resultsItr == document.end() || !resultsItr->is_array())
            {
                return {};
            }

            std::vector<Point> points;

            for (const auto& result : *resultsItr)
            {
                if (!result.is_object())
                {
                    continue;
                }

                checkError(result);

                const auto seriesItr = result.find("series");
                if (seriesItr == result.end() || !seriesItr->is_array())
                {
                    continue;
                }

                for (const auto& series : *seriesItr)
                {
                    auto seriesPoints = processSeries(series);
                    points.reserve(points.size() + seriesPoints.size());
                    points.insert(points.end(), std::make_move_iterator(seriesPoints.begin()), std::make_move_iterator(seriesPoints.end()));
                }
            }
            return points;
        }
        catch (const nlohmann::json::exception& e)
        {
            throw std::runtime_error(std::format("InfluxDB query: JSON parsing failed: {}", e.what()));
        }
    }

    std::unique_ptr<Transport> withUdpTransport(const http::url& uri)
    {
        return std::make_unique<transports::UDP>(uri.host, uri.port);
    }

    std::unique_ptr<Transport> withTcpTransport(const http::url& uri)
    {
        return std::make_unique<transports::TCP>(uri.host, uri.port);
    }

    std::unique_ptr<Transport> withUnixSocketTransport(const http::url& uri)
    {
        return std::make_unique<transports::UnixSocket>(uri.path);
    }
}
