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

#include "InfluxDBFactory.h"
#include <functional>
#include <string>
#include <memory>
#include <map>
#include "UriParser.h"
#include "HTTP.h"
#include "InfluxDBException.h"
#include "BoostSupport.h"

namespace influxdb
{
    namespace internal
    {
        std::unique_ptr<Transport> withHttpTransport(const http::url& uri)
        {
            auto transport = std::make_unique<transports::HTTP>(uri.url);
            if (!uri.user.empty())
            {
                transport->setBasicAuthentication(uri.user, uri.password);
            }
            return transport;
        }

    }

    std::unique_ptr<Transport> InfluxDBFactory::GetTransport(const std::string& url)
    {
        static const std::map<std::string, std::function<std::unique_ptr<Transport>(const http::url&)>> map = {
            {"udp", internal::withUdpTransport},
            {"tcp", internal::withTcpTransport},
            {"http", internal::withHttpTransport},
            {"https", internal::withHttpTransport},
            {"unix", internal::withUnixSocketTransport},
        };

        auto urlCopy = url;
        http::url parsedUrl{http::ParseHttpUrl(urlCopy)};

        const auto iterator = map.find(parsedUrl.protocol);
        if (iterator == map.end())
        {
            throw InfluxDBException("Unrecognized backend " + parsedUrl.protocol);
        }

        return iterator->second(parsedUrl);
    }

    std::unique_ptr<InfluxDB> InfluxDBFactory::Get(const std::string& url)
    {
        return std::make_unique<InfluxDB>(InfluxDBFactory::GetTransport(url));
    }

    std::unique_ptr<InfluxDB> InfluxDBFactory::Get(const std::string& url, const Proxy& proxy)
    {
        auto transport = InfluxDBFactory::GetTransport(url);
        transport->setProxy(proxy);
        return std::make_unique<InfluxDB>(std::move(transport));
    }

} // namespace influxdb
