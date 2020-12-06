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

#include "InfluxDBFactory.h"
#include <functional>
#include <string>
#include <memory>
#include <map>
#include "UriParser.h"
#include "HTTP.h"
#include "InfluxDBException.h"

#ifdef INFLUXDB_WITH_BOOST

#include "UDP.h"
#include "UnixSocket.h"

#endif

namespace influxdb
{

#ifdef INFLUXDB_WITH_BOOST

std::unique_ptr<Transport> withUdpTransport(const http::url &uri)
{
  return std::make_unique<transports::UDP>(uri.host, uri.port);
}

std::unique_ptr<Transport> withUnixSocketTransport(const http::url &uri)
{
  return std::make_unique<transports::UnixSocket>(uri.path);
}

#else
std::unique_ptr<Transport> withUdpTransport([[maybe_unused]] const http::url& uri) {
  throw InfluxDBException("InfluxDBFactory", "UDP transport requires Boost");
}

std::unique_ptr<Transport> withUnixSocketTransport([[maybe_unused]] const http::url& uri) {
  throw InfluxDBException("InfluxDBFactory", "Unix socket transport requires Boost");
}
#endif

std::unique_ptr<Transport> withHttpTransport(const http::url &uri)
{
  auto transport = std::make_unique<transports::HTTP>(uri.url);
  if (!uri.user.empty())
  {
    transport->enableBasicAuth(uri.user + ":" + uri.password);
  }
  return transport;
}

std::unique_ptr<Transport> InfluxDBFactory::GetTransport(const std::string& url)
{
  static const std::map<std::string, std::function<std::unique_ptr<Transport>(const http::url &)>> map = {
    {"udp",   withUdpTransport},
    {"http",  withHttpTransport},
    {"https", withHttpTransport},
    {"unix",  withUnixSocketTransport},
  };

  auto urlCopy = url;
  http::url parsedUrl = http::ParseHttpUrl(urlCopy);
  if (parsedUrl.protocol.empty())
  {
    throw InfluxDBException(__func__, "Ill-formed URI");
  }

  const auto iterator = map.find(parsedUrl.protocol);
  if (iterator == map.end())
  {
    throw InfluxDBException(__func__, "Unrecognized backend " + parsedUrl.protocol);
  }

  return iterator->second(parsedUrl);
}

std::unique_ptr<InfluxDB> InfluxDBFactory::Get(const std::string& url)
{
  return std::make_unique<InfluxDB>(InfluxDBFactory::GetTransport(url));
}

} // namespace influxdb
