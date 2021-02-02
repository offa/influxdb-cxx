// MIT License
//
// Copyright (c) 2020-2021 offa
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

#include "LineProtocol.h"

namespace influxdb
{
    LineProtocol::LineProtocol()
        : LineProtocol(std::string{})
    {
    }

    LineProtocol::LineProtocol(const std::string& tags)
        : globalTags(tags)
    {
    }

    std::string LineProtocol::format(const Point& point)
    {
        std::string line{point.getName()};

        if (!globalTags.empty())
        {
            line.append(",");
            line.append(globalTags);
        }

        if (!point.getTags().empty())
        {
            line.append(",");
            line.append(point.getTags());
        }

        if (!point.getFields().empty())
        {
            line.append(" ").append(point.getFields());
        }

        return line.append(" ")
            .append(std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(point.getTimestamp().time_since_epoch()).count()));
    }
}
