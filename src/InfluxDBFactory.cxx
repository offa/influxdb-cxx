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
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#include "InfluxDB/InfluxDBFactory.h"
#include <memory>
#include <string>
#include <string_view>
#include "InfluxDB/InfluxDBException.h"
#include "UriParser.h"
#include "EndpointTransport.h"
#include "HTTP.h"
#include "BoostSupport.h"

namespace influxdb
{
    namespace internal
    {
        namespace
        {
            std::string makeHttpUrl(const Endpoint& endpoint, std::string_view originalUri)
            {
                std::string url;
                url.reserve(endpoint.host.size() + endpoint.path.size() + 32);

                url += endpoint.scheme == Scheme::Https ? "https" : "http";
                url += "://";

                if (endpoint.host.find(':') != std::string::npos)
                {
                    url += '[';
                    url += endpoint.host;
                    url += ']';
                }
                else
                {
                    url += endpoint.host;
                }

                url += ':';
                url += std::to_string(endpoint.port);
                url += endpoint.path;

                const auto queryPosition = originalUri.find('?');
                if (queryPosition != std::string_view::npos)
                {
                    url.append(originalUri.substr(queryPosition));
                }

                return url;
            }

            void applyAuthentication(transports::HTTP& transport, const Authentication& authentication)
            {
                switch (authentication.type)
                {
                    case AuthenticationType::Basic:
                        transport.setBasicAuthentication(authentication.username, authentication.password);
                        break;
                    case AuthenticationType::ApiKey:
                        transport.setAuthToken(authentication.apiKey);
                        break;
                    case AuthenticationType::None:
                        break;
                }
            }
        }

        std::unique_ptr<Transport> withHttpTransport(const Endpoint& endpoint, std::string_view originalUri)
        {
            if (endpoint.scheme != Scheme::Http && endpoint.scheme != Scheme::Https)
            {
                throw InfluxDBException{"Invalid endpoint: HTTP transport requires http or https"};
            }

            auto transport = std::make_unique<transports::HTTP>(makeHttpUrl(endpoint, originalUri));
            applyAuthentication(*transport, endpoint.authentication);
            return transport;
        }
    }

    std::unique_ptr<Transport> InfluxDBFactory::GetTransport(const std::string& url)
    {
        const auto endpoint = internal::parseEndpoint(url);

        switch (endpoint.scheme)
        {
            case internal::Scheme::Http:
            case internal::Scheme::Https:
                return internal::withHttpTransport(endpoint, url);
            case internal::Scheme::Tcp:
                return internal::withTcpTransport(endpoint);
            case internal::Scheme::Udp:
                return internal::withUdpTransport(endpoint);
            case internal::Scheme::Unix:
                return internal::withUnixSocketTransport(endpoint);
        }

        throw InfluxDBException{"Unrecognized backend"};
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
