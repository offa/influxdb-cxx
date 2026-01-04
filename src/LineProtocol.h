// MIT License
//
// Copyright (c) 2020-2026 offa
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

#pragma once

#include "InfluxDB/Point.h"
#include "InfluxDB/TimePrecision.h"

#include <string>

namespace influxdb
{
    class LineProtocol
    {
    public:
        // Caller must ensure that the tags string is correctly escaped
        LineProtocol(const std::string& tags, TimePrecision precision);

        std::string format(const Point& point) const;

        enum class ElementType
        {
            Measurement,
            TagKey,
            TagValue,
            FieldKey,
            FieldValue
        };

        // Escapes special characters in a string element according to the
        // InfluxDB line protocol specification.
        // https://docs.influxdata.com/influxdb/cloud/reference/syntax/line-protocol/#special-characters
        static std::string EscapeStringElement(ElementType type, std::string_view stringElement);

    private:
        std::string globalTags;
        TimePrecision timePrecision;
    };
}
