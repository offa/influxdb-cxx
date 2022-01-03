// MIT License
//
// Copyright (C) 2020-2022  offa
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
#include "InfluxDBException.h"

namespace influxdb::internal
{
    std::vector<Point> queryImpl([[maybe_unused]] Transport* transport, [[maybe_unused]] const std::string& query)
    {
        throw InfluxDBException("InfluxDB", "Query requires Boost");
    }

    std::unique_ptr<Transport> withUdpTransport([[maybe_unused]] const http::url& uri)
    {
        throw InfluxDBException("InfluxDBFactory", "UDP transport requires Boost");
    }

    std::unique_ptr<Transport> withUnixSocketTransport([[maybe_unused]] const http::url& uri)
    {
        throw InfluxDBException("InfluxDBFactory", "Unix socket transport requires Boost");
    }
}
