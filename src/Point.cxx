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

#include "Point.h"
#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip>

namespace influxdb
{

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

Point::Point(const std::string& measurement) : mValue({}),
  mMeasurement(measurement), mTimestamp(Point::getCurrentTimestamp()), mTags({}), mFields({})
{
}

Point&& Point::addField(std::string_view name, const std::variant<int, long long int, std::string, double>& value)
{
  if (name.empty())
  {
    return std::move(*this);
  }

  std::stringstream convert;
  convert << std::setprecision(floatsPrecision);
  if (!mFields.empty()) convert << ",";

  convert << name << "=";
  std::visit(overloaded {
    [&convert](int v) { convert << v << 'i'; },
    [&convert](long long int v) { convert << v << 'i'; },
    [&convert](double v) { convert  << std::fixed << v; },
    [&convert](const std::string& v) { convert << '"' << v << '"'; },
    }, value);
  mFields += convert.str();
  return std::move(*this);
}

Point&& Point::addTag(std::string_view key, std::string_view value)
{
  if (key.empty() || value.empty())
  {
    return std::move(*this);
  }
  mTags += ",";
  mTags += key;
  mTags += "=";
  mTags += value;
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
    std::string line{mMeasurement};

    if (!mTags.empty())
    {
        line.append(mTags);
    }

    if (!mFields.empty())
    {
        line.append(" ").append(mFields);
    }

    return line.append(" ")
        .append(std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(mTimestamp.time_since_epoch()).count()));
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
  return mFields;
}

std::string Point::getTags() const
{
    if (mTags.empty())
    {
        return "";
    }
    return mTags.substr(1, mTags.size());
}

} // namespace influxdb
