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

#include "InfluxDB/InfluxDBBuilder.h"
#include "HTTP.h"

namespace influxdb
{
    InfluxDBBuilder::InfluxDBBuilder(std::unique_ptr<Transport> impl)
        : transport(std::move(impl))
    {
    }

    std::unique_ptr<InfluxDB> InfluxDBBuilder::connect()
    {
        return std::make_unique<InfluxDB>(std::move(transport));
    }

    InfluxDBBuilder&& InfluxDBBuilder::setBasicAuthentication(const std::string& user, const std::string& pass)
    {
        dynamic_cast<transports::HTTP&>(*transport).setBasicAuthentication(user, pass);
        return std::move(*this);
    }

    InfluxDBBuilder&& InfluxDBBuilder::setAuthToken(const std::string& token)
    {
        dynamic_cast<transports::HTTP&>(*transport).setAuthToken(token);
        return std::move(*this);
    }

    InfluxDBBuilder&& InfluxDBBuilder::setProxy(const Proxy& proxy)
    {
        dynamic_cast<transports::HTTP&>(*transport).setProxy(proxy);
        return std::move(*this);
    }

    InfluxDBBuilder&& InfluxDBBuilder::setTimeout(std::chrono::milliseconds timeout)
    {
        dynamic_cast<transports::HTTP&>(*transport).setTimeout(timeout);
        return std::move(*this);
    }

    InfluxDBBuilder&& InfluxDBBuilder::setVerifyCertificate(bool verify)
    {
        dynamic_cast<transports::HTTP&>(*transport).setVerifyCertificate(verify);
        return std::move(*this);
    }

    InfluxDBBuilder InfluxDBBuilder::http(const std::string& url)
    {
        return InfluxDBBuilder{std::make_unique<transports::HTTP>(url)};
    }


}
