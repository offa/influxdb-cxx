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

#include "HTTP.h"
#include "InfluxDBException.h"

namespace influxdb::transports
{
    namespace
    {
        void checkResponse(const cpr::Response& resp)
        {
            if (resp.error)
            {
                throw ConnectionError(__func__, resp.error.message);
            }
            if (resp.status_code == 404)
            {
                throw NonExistentDatabase(__func__, "Nonexistent database: " + std::to_string(resp.status_code));
            }
            if (cpr::status::is_client_error(resp.status_code))
            {
                throw BadRequest(__func__, "Bad request: " + std::to_string(resp.status_code));
            }
            if (cpr::status::is_server_error(resp.status_code))
            {
                throw ServerError(__func__, "Influx server error: " + std::to_string(resp.status_code));
            }
            if (!cpr::status::is_success(resp.status_code))
            {
                throw ConnectionError(__func__, "(" + std::to_string(resp.status_code) + ") " + resp.reason);
            }
        }

        std::string parseUrl(const std::string& url)
        {
            const auto questionMarkPosition = url.find('?');

            if (questionMarkPosition == std::string::npos)
            {
                return url;
            }
            if (url.at(questionMarkPosition - 1) == '/')
            {
                return url.substr(0, questionMarkPosition - 1);
            }
            return url.substr(0, questionMarkPosition);
        }

        std::string parseDatabaseName(const std::string& url)
        {
            const auto dbParameterPosition = url.find("?db=");

            if (dbParameterPosition == std::string::npos)
            {
                throw InfluxDBException{__func__, "Database not specified"};
            }
            return url.substr(dbParameterPosition + 4);
        }
    }


    HTTP::HTTP(const std::string& url)
        : mInfluxDbServiceUrl(parseUrl(url)), mDatabaseName(parseDatabaseName(url))
    {
        session.SetTimeout(cpr::Timeout{std::chrono::seconds{10}});
        session.SetConnectTimeout(cpr::ConnectTimeout{std::chrono::seconds{10}});
    }

    std::string HTTP::query(const std::string& query)
    {
        session.SetUrl(cpr::Url{mInfluxDbServiceUrl + "/query"});
        session.SetParameters(cpr::Parameters{{"db", mDatabaseName}, {"q", query}});

        const auto response = session.Get();
        checkResponse(response);

        return response.text;
    }

    void HTTP::enableBasicAuth(const std::string& auth)
    {
        const auto delim = auth.find(':');
        session.SetAuth(cpr::Authentication{auth.substr(0, delim), auth.substr(delim + 1), cpr::AuthMode::BASIC});
    }

    void HTTP::send(std::string&& lineprotocol)
    {
        session.SetUrl(cpr::Url{mInfluxDbServiceUrl + "/write"});
        session.SetHeader(cpr::Header{{"Content-Type", "application/json"}});
        session.SetParameters(cpr::Parameters{{"db", mDatabaseName}});
        session.SetBody(cpr::Body{lineprotocol});

        const auto response = session.Post();
        checkResponse(response);
    }

    std::string HTTP::databaseName() const
    {
        return mDatabaseName;
    }

    std::string HTTP::influxDbServiceUrl() const
    {
        return mInfluxDbServiceUrl;
    }

    void HTTP::setProxy(const Proxy& proxy)
    {
        session.SetProxies(cpr::Proxies{{"http", proxy.getProxy()}, {"https", proxy.getProxy()}});

        if (const auto& auth = proxy.getAuthentication(); auth.has_value())
        {
            session.SetProxyAuth(cpr::ProxyAuthentication{{"http", cpr::EncodedAuthentication{auth->user, auth->password}},
                                                          {"https", cpr::EncodedAuthentication{auth->user, auth->password}}});
        }
    }

    std::string HTTP::execute(const std::string& cmd)
    {
        session.SetUrl(cpr::Url{mInfluxDbServiceUrl + "/query"});
        session.SetParameters(cpr::Parameters{{"db", mDatabaseName}, {"q", cmd}});

        const auto response = session.Get();
        checkResponse(response);

        return response.text;
    }

    void HTTP::createDatabase()
    {
        session.SetUrl(cpr::Url{mInfluxDbServiceUrl + "/query"});
        session.SetParameters(cpr::Parameters{{"q", "CREATE DATABASE " + mDatabaseName}});

        const auto response = session.Post();
        checkResponse(response);
    }

} // namespace influxdb
