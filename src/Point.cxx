// MIT License
//
// Copyright (c) 2020-2022 offa
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

#include "Point.h"
#include "LineProtocol.h"
#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip>

namespace influxdb
{

    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    Point::Point(const std::string& measurement)
        : mMeasurement(measurement), mTimestamp(Point::getCurrentTimestamp()), mTags({}), mFields({})
    {
    }

    Point&& Point::addField(std::string_view name, const std::variant<int, long long int, std::string, double>& value)
    {
        if (name.empty())
        {
            return std::move(*this);
        }

        mFields.emplace_back(std::make_pair(name, value));
        return std::move(*this);
    }

    Point&& Point::addTag(std::string_view key, std::string_view value)
    {
        if (key.empty() || value.empty())
        {
            return std::move(*this);
        }

        mTags.emplace_back(std::make_pair(key, value));
        return std::move(*this);
    }

    Point&& Point::setTimestamp(std::chrono::time_point<std::chrono::system_clock> timestamp)
    {
        mTimestamp = timestamp;
        return std::move(*this);
    }

    auto Point::getCurrentTimestamp() -> decltype(std::chrono::system_clock::now())
    {
        return std::chrono::system_clock::now();
    }

    std::string Point::toLineProtocol() const
    {
        LineProtocol formatter;
        return formatter.format(*this);
    }

    std::string Point::getName() const
    {
        return mMeasurement;
    }

    std::chrono::time_point<std::chrono::system_clock> Point::getTimestamp() const
    {
        return mTimestamp;
    }

    std::string Point::getFields() const
    {
        std::string fields;

        for (const auto& field : mFields)
        {
            std::stringstream convert;
            convert << std::setprecision(floatsPrecision);

            if (!fields.empty())
            {
                convert << ",";
            }

            convert << field.first << "=";
            std::visit(overloaded{
                           [&convert](int v)
                           { convert << v << 'i'; },
                           [&convert](long long int v)
                           { convert << v << 'i'; },
                           [&convert](double v)
                           { convert << std::fixed << v; },
                           [&convert](const std::string& v)
                           { convert << '"' << v << '"'; },
                       },
                       field.second);

            fields += convert.str();
        }

        return fields;
    }

    std::string Point::getTags() const
    {
        if (mTags.empty())
        {
            return "";
        }

        std::string tags;
        for (const auto& tag : mTags)
        {
            tags += ",";
            tags += tag.first;
            tags += "=";
            tags += tag.second;
        }

        return tags.substr(1, tags.size());
    }

    void Point::setLine(std::string line)
    {
        this.mLine = line;
    }

    std::string Point::getLine()
    {
        return mLine;
    }

} // namespace influxdb
