// MIT License
//
// Copyright (c) 2020-2025 offa
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

#ifndef INFLUXDATA_INFLUXDBBUILDER_H
#define INFLUXDATA_INFLUXDBBUILDER_H

#include "InfluxDB/InfluxDB.h"
#include "InfluxDB/Transport.h"
#include "InfluxDB/Proxy.h"
#include "InfluxDB/influxdb_export.h"
#include <chrono>

namespace influxdb
{
    class INFLUXDB_EXPORT InfluxDBBuilder
    {
    public:
        std::unique_ptr<InfluxDB> connect();

        InfluxDBBuilder&& setBasicAuthentication(const std::string& user, const std::string& pass);
        InfluxDBBuilder&& setAuthToken(const std::string& token);
        InfluxDBBuilder&& setProxy(const Proxy& proxy);
        InfluxDBBuilder&& setTimeout(std::chrono::milliseconds timeout);
        InfluxDBBuilder&& setVerifyCertificate(bool verify);

        static InfluxDBBuilder http(const std::string& url);

    private:
        explicit InfluxDBBuilder(std::unique_ptr<Transport> impl);

        std::unique_ptr<Transport> transport;
    };
}

#endif // INFLUXDATA_INFLUXDBBUILDER_H
