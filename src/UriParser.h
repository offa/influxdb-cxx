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

#ifndef INFLUXDATA_URIPARSER_H
#define INFLUXDATA_URIPARSER_H

#include "InfluxDB/InfluxDBException.h"
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

namespace influxdb::internal
{
    enum class Scheme
    {
        Http,
        Https,
        Tcp,
        Udp,
        Unix
    };

    enum class AuthenticationType
    {
        None,
        Basic,
        ApiKey
    };

    struct Authentication
    {
        AuthenticationType type = AuthenticationType::None;

        // Basic authentication only.
        std::string username;
        std::string password;

        // API key authentication only.
        std::string apiKey;
    };

    struct Endpoint
    {
        Scheme scheme = Scheme::Http;

        // DNS name, IPv4 address, or IPv6 address without brackets.
        // Empty for Unix sockets.
        std::string host;

        // HTTP/HTTPS: 8086 when omitted.
        // TCP/UDP: required.
        // Unix: unused.
        std::uint16_t port = 0;

        // HTTP/HTTPS: optional base path.
        // Unix: absolute socket path.
        // TCP/UDP: must be empty.
        std::string path;

        Authentication authentication;
    };

    namespace detail
    {
        inline Scheme parseScheme(std::string_view scheme)
        {
            if (scheme == "http")
            {
                return Scheme::Http;
            }
            if (scheme == "https")
            {
                return Scheme::Https;
            }
            if (scheme == "tcp")
            {
                return Scheme::Tcp;
            }
            if (scheme == "udp")
            {
                return Scheme::Udp;
            }
            if (scheme == "unix")
            {
                return Scheme::Unix;
            }
            throw InfluxDBException{"Invalid endpoint: unsupported scheme"};
        }

        inline std::uint16_t parsePort(std::string_view port)
        {
            unsigned int value = 0;
            const auto result = std::from_chars(port.data(), port.data() + port.size(), value);

            if (result.ec != std::errc{} || result.ptr != port.data() + port.size() || value == 0 || value > 65535)
            {
                throw InfluxDBException{"Invalid endpoint: invalid port"};
            }

            return static_cast<std::uint16_t>(value);
        }

        inline void parseAuthentication(std::string_view authority, Authentication& authentication, std::string_view& hostPort)
        {
            const auto at = authority.find('@');
            if (at == std::string_view::npos)
            {
                hostPort = authority;
                return;
            }

            if (authority.find('@', at + 1) != std::string_view::npos)
            {
                throw InfluxDBException{"Invalid endpoint: malformed authentication"};
            }

            const auto userInfo = authority.substr(0, at);
            hostPort = authority.substr(at + 1);

            if (userInfo.empty())
            {
                throw InfluxDBException{"Invalid endpoint: malformed authentication"};
            }

            const auto colon = userInfo.find(':');
            if (colon == std::string_view::npos)
            {
                authentication.type = AuthenticationType::ApiKey;
                authentication.apiKey = std::string{userInfo};
                return;
            }

            const auto username = userInfo.substr(0, colon);
            const auto password = userInfo.substr(colon + 1);

            if (username.empty() || password.empty())
            {
                throw InfluxDBException{"Invalid endpoint: malformed authentication"};
            }

            authentication.type = AuthenticationType::Basic;
            authentication.username = std::string{username};
            authentication.password = std::string{password};
        }

        inline void parseHostPort(std::string_view hostPort, Scheme scheme, Endpoint& endpoint)
        {
            std::string_view host;
            std::string_view port;
            bool hasPort = false;

            if (!hostPort.empty() && hostPort.front() == '[')
            {
                const auto closingBracket = hostPort.find(']');
                if (closingBracket == std::string_view::npos)
                {
                    throw InfluxDBException{"Invalid endpoint: malformed IPv6 address"};
                }

                host = hostPort.substr(1, closingBracket - 1);
                if (host.empty() || host.find(':') == std::string_view::npos)
                {
                    throw InfluxDBException{"Invalid endpoint: malformed IPv6 address"};
                }

                const auto remainder = hostPort.substr(closingBracket + 1);
                if (!remainder.empty())
                {
                    if (remainder.front() != ':')
                    {
                        throw InfluxDBException{"Invalid endpoint: malformed IPv6 address"};
                    }
                    port = remainder.substr(1);
                    hasPort = true;
                }
            }
            else
            {
                const auto colon = hostPort.find(':');
                if (colon == std::string_view::npos)
                {
                    host = hostPort;
                }
                else
                {
                    host = hostPort.substr(0, colon);
                    port = hostPort.substr(colon + 1);
                    hasPort = true;

                    if (port.find(':') != std::string_view::npos)
                    {
                        throw InfluxDBException{"Invalid endpoint: IPv6 address must be bracketed"};
                    }
                }
            }

            if (host.empty())
            {
                throw InfluxDBException{"Invalid endpoint: missing host"};
            }

            if (host.find_first_of(" \t\r\n/?#@[]") != std::string_view::npos)
            {
                throw InfluxDBException{"Invalid endpoint: invalid host"};
            }

            endpoint.host = std::string{host};

            if (hasPort)
            {
                if (port.empty())
                {
                    throw InfluxDBException{"Invalid endpoint: invalid port"};
                }
                endpoint.port = parsePort(port);
                return;
            }

            switch (scheme)
            {
                case Scheme::Http:
                case Scheme::Https:
                    endpoint.port = 8086;
                    break;
                case Scheme::Tcp:
                    throw InfluxDBException{"Invalid endpoint: TCP requires a port"};
                case Scheme::Udp:
                    throw InfluxDBException{"Invalid endpoint: UDP requires a port"};
                case Scheme::Unix:
                    break;
            }
        }
    }

    inline Endpoint parseEndpoint(std::string_view uri)
    {
        constexpr std::string_view separator{"://"};
        const auto separatorPosition = uri.find(separator);

        if (separatorPosition == std::string_view::npos || separatorPosition == 0)
        {
            throw InfluxDBException{"Invalid endpoint: missing scheme separator"};
        }

        const auto scheme = detail::parseScheme(uri.substr(0, separatorPosition));
        const auto rest = uri.substr(separatorPosition + separator.size());

        if (rest.find('#') != std::string_view::npos)
        {
            throw InfluxDBException{"Invalid endpoint: fragments are not supported"};
        }

        Endpoint endpoint;
        endpoint.scheme = scheme;

        if (scheme == Scheme::Unix)
        {
            if (rest.find('?') != std::string_view::npos)
            {
                throw InfluxDBException{"Invalid endpoint: query strings are not supported for this scheme"};
            }

            if (rest.empty() || rest.front() != '/')
            {
                throw InfluxDBException{"Invalid endpoint: Unix socket path must be absolute"};
            }

            endpoint.path = std::string{rest};
            return endpoint;
        }

        std::string_view endpointPart = rest;
        if (scheme == Scheme::Http || scheme == Scheme::Https)
        {
            const auto queryPosition = rest.find('?');
            if (queryPosition != std::string_view::npos)
            {
                endpointPart = rest.substr(0, queryPosition);
            }
        }
        else if (rest.find('?') != std::string_view::npos)
        {
            throw InfluxDBException{"Invalid endpoint: query strings are not supported for this scheme"};
        }

        const auto pathPosition = endpointPart.find('/');
        const auto authority = endpointPart.substr(0, pathPosition);
        const auto path = pathPosition == std::string_view::npos ? std::string_view{} : endpointPart.substr(pathPosition);

        if (authority.empty())
        {
            throw InfluxDBException{"Invalid endpoint: missing host"};
        }

        // This client does not support '@' in HTTP paths. Rejecting it prevents
        // an authentication-looking path from being silently accepted.
        if (path.find('@') != std::string_view::npos)
        {
            throw InfluxDBException{"Invalid endpoint: malformed authentication"};
        }

        if ((scheme == Scheme::Tcp || scheme == Scheme::Udp) && !path.empty())
        {
            throw InfluxDBException{"Invalid endpoint: path is not supported for this scheme"};
        }

        std::string_view hostPort;
        detail::parseAuthentication(authority, endpoint.authentication, hostPort);
        detail::parseHostPort(hostPort, scheme, endpoint);
        endpoint.path = std::string{path};

        return endpoint;
    }
}

#endif
