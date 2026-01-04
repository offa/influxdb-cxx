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

///
/// \author Adam Wegrzynek
///

#ifndef INFLUXDATA_POINT_H
#define INFLUXDATA_POINT_H

#include <string>
#include <string_view>
#include <chrono>
#include <variant>
#include <deque>

#include "InfluxDB/influxdb_export.h"

namespace influxdb
{

    static inline constexpr int defaultFloatsPrecision{18};

    /// \brief Represents a point
    class INFLUXDB_EXPORT Point
    {
    public:
        /// Constructs point based on measurement name
        explicit Point(const std::string& measurement);

        /// Adds a tags
        Point&& addTag(std::string_view key, std::string_view value);

        /// Adds field
        using FieldValue = std::variant<int, long long int, std::string, double, bool, unsigned int, unsigned long long int>;
        Point&& addField(std::string_view name, const FieldValue& value);

        /// Sets custom timestamp
        Point&& setTimestamp(std::chrono::time_point<std::chrono::system_clock> timestamp);

        /// Name getter
        std::string getName() const;

        /// Timestamp getter
        std::chrono::time_point<std::chrono::system_clock> getTimestamp() const;

        /// Fields getter
        std::string getFields() const;

        /// Get Field Set
        using FieldSet = std::deque<std::pair<std::string, FieldValue>>;
        const FieldSet& getFieldSet() const;

        /// Tags getter
        std::string getTags() const;

        /// Get Tag Set
        using TagSet = std::deque<std::pair<std::string, std::string>>;
        const TagSet& getTagSet() const;

        /// Precision for float fields
        static inline int floatsPrecision{defaultFloatsPrecision};

    protected:
        /// A name
        std::string mMeasurement;

        /// A timestamp
        std::chrono::time_point<std::chrono::system_clock> mTimestamp;

        //// Tags
        TagSet mTags;

        //// Fields
        FieldSet mFields;
    };

} // namespace influxdb

#endif // INFLUXDATA_POINT_H
