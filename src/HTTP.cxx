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
#include <mutex>


namespace influxdb::transports
{
    namespace
    {
        size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
        {
            static_cast<std::string*>(userp)->append(static_cast<char*>(contents), size * nmemb);
            return size * nmemb;
        }

        size_t noopWriteCallBack([[maybe_unused]] char* ptr, size_t size,
                                 size_t nmemb, [[maybe_unused]] void* userdata)
        {
            return size * nmemb;
        }

        void setConnectionOptions(CURL* handle)
        {
            curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 10);
            curl_easy_setopt(handle, CURLOPT_TIMEOUT, 10);
            curl_easy_setopt(handle, CURLOPT_TCP_KEEPIDLE, 120L);
            curl_easy_setopt(handle, CURLOPT_TCP_KEEPINTVL, 60L);
            curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, noopWriteCallBack);
        }

        CURL* createReadHandle()
        {
            if (CURL* readHandle = curl_easy_init(); readHandle != nullptr)
            {
                setConnectionOptions(readHandle);
                curl_easy_setopt(readHandle, CURLOPT_WRITEFUNCTION, WriteCallback);
                return readHandle;
            }

            throw InfluxDBException{__func__, "Failed to initialize write handle"};
        }

        CURL* createWriteHandle(const std::string& url)
        {
            if (CURL* writeHandle = curl_easy_init(); writeHandle != nullptr)
            {
                setConnectionOptions(writeHandle);
                curl_easy_setopt(writeHandle, CURLOPT_URL, url.c_str());
                curl_easy_setopt(writeHandle, CURLOPT_POST, 1);
                return writeHandle;
            }

            throw InfluxDBException{__func__, "Failed to initialize write handle"};
        }
    }

    HTTP::HTTP(const std::string& url)
    {
        initCurl(url);
        initCurlRead(url);
        obtainInfluxServiceUrl(url);
        obtainDatabaseName(url);
    }

    HTTP::~HTTP()
    {
        curl_easy_cleanup(writeHandle);
        curl_easy_cleanup(readHandle);
    }

    void HTTP::initCurl(const std::string& url)
    {
        static std::once_flag curl_global_inited_flag;
        std::call_once(curl_global_inited_flag, []()
                       {
            if (const CURLcode globalInitResult = curl_global_init(CURL_GLOBAL_ALL); globalInitResult != CURLE_OK)
            {
              throw InfluxDBException(__func__, curl_easy_strerror(globalInitResult));
            }; });
        std::string writeUrl = url;
        auto position = writeUrl.find('?');
        if (position == std::string::npos)
        {
            throw InfluxDBException(__func__, "Database not specified");
        }
        if (writeUrl.at(position - 1) != '/')
        {
            writeUrl.insert(position, "/write");
        }
        else
        {
            writeUrl.insert(position, "write");
        }
        writeHandle = createWriteHandle(writeUrl);
    }

    void HTTP::initCurlRead(const std::string& url)
    {
        mReadUrl = url + "&q=";
        const auto pos = mReadUrl.find('?');
        std::string cmd{"query"};

        if (mReadUrl[pos - 1] != '/')
        {
            cmd.insert(0, 1, '/');
        }

        mReadUrl.insert(pos, cmd);
        readHandle = createReadHandle();
    }

    std::string HTTP::query(const std::string& query)
    {
        std::string buffer;
        char* encodedQuery = curl_easy_escape(readHandle, query.c_str(), static_cast<int>(query.size()));
        auto fullUrl = mReadUrl + std::string(encodedQuery);
        curl_free(encodedQuery);
        curl_easy_setopt(readHandle, CURLOPT_URL, fullUrl.c_str());
        curl_easy_setopt(readHandle, CURLOPT_WRITEDATA, &buffer);
        const CURLcode response = curl_easy_perform(readHandle);
        long responseCode{0};
        curl_easy_getinfo(readHandle, CURLINFO_RESPONSE_CODE, &responseCode);
        treatCurlResponse(response, responseCode);
        return buffer;
    }

    void HTTP::enableBasicAuth(const std::string& auth)
    {
        curl_easy_setopt(writeHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(writeHandle, CURLOPT_USERPWD, auth.c_str());
        curl_easy_setopt(readHandle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(readHandle, CURLOPT_USERPWD, auth.c_str());
    }

    void HTTP::send(std::string&& lineprotocol)
    {
        curl_easy_setopt(writeHandle, CURLOPT_POSTFIELDS, lineprotocol.c_str());
        curl_easy_setopt(writeHandle, CURLOPT_POSTFIELDSIZE, static_cast<long>(lineprotocol.length()));
        const CURLcode response = curl_easy_perform(writeHandle);
        long responseCode{0};
        curl_easy_getinfo(writeHandle, CURLINFO_RESPONSE_CODE, &responseCode);
        treatCurlResponse(response, responseCode);
    }

    void HTTP::treatCurlResponse(const CURLcode& response, long responseCode) const
    {
        if (response != CURLE_OK)
        {
            throw ConnectionError(__func__, curl_easy_strerror(response));
        }
        //
        // Influx API response codes:
        // https://docs.influxdata.com/influxdb/v1.7/tools/api/#status-codes-and-responses-2
        //
        if (responseCode == 404)
        {
            throw NonExistentDatabase(__func__, "Nonexistent database: " + std::to_string(responseCode));
        }
        if ((responseCode >= 400) && (responseCode < 500))
        {
            throw BadRequest(__func__, "Bad request: " + std::to_string(responseCode));
        }
        if (responseCode >= 500)
        {
            throw ServerError(__func__, "Influx server error: " + std::to_string(responseCode));
        }
    }

    void HTTP::obtainInfluxServiceUrl(const std::string& url)
    {
        const auto questionMarkPosition = url.find('?');

        if (url.at(questionMarkPosition - 1) == '/')
        {
            mInfluxDbServiceUrl = url.substr(0, questionMarkPosition - 1);
        }
        else
        {
            mInfluxDbServiceUrl = url.substr(0, questionMarkPosition);
        }
    }

    void HTTP::obtainDatabaseName(const std::string& url)
    {
        const auto dbParameterPosition = url.find("db=");
        mDatabaseName = url.substr(dbParameterPosition + 3);
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
        auto proxyServer = proxy.getProxy();
        curl_easy_setopt(writeHandle, CURLOPT_PROXY, proxyServer.c_str());
        curl_easy_setopt(readHandle, CURLOPT_PROXY, proxyServer.c_str());

        if (auto auth = proxy.getAuthentication(); auth.has_value())
        {
            curl_easy_setopt(writeHandle, CURLOPT_PROXYUSERNAME, auth->user.c_str());
            curl_easy_setopt(writeHandle, CURLOPT_PROXYPASSWORD, auth->password.c_str());

            curl_easy_setopt(readHandle, CURLOPT_PROXYUSERNAME, auth->user.c_str());
            curl_easy_setopt(readHandle, CURLOPT_PROXYPASSWORD, auth->password.c_str());
        }
    }

    std::string HTTP::execute(const std::string& cmd)
    {
        std::string buffer;
        char* encodedQuery = curl_easy_escape(readHandle, cmd.c_str(), static_cast<int>(cmd.size()));
        auto fullUrl = mInfluxDbServiceUrl + "/query?q=" + std::string(encodedQuery);
        curl_free(encodedQuery);
        curl_easy_setopt(readHandle, CURLOPT_URL, fullUrl.c_str());
        curl_easy_setopt(readHandle, CURLOPT_WRITEDATA, &buffer);
        const CURLcode response = curl_easy_perform(readHandle);
        long responseCode{0};
        curl_easy_getinfo(readHandle, CURLINFO_RESPONSE_CODE, &responseCode);
        treatCurlResponse(response, responseCode);
        return buffer;
    }

    void HTTP::createDatabase()
    {
        const std::string createUrl = mInfluxDbServiceUrl + "/query";
        const std::string postFields = "q=CREATE DATABASE " + mDatabaseName;

        CURL* createHandle = createWriteHandle(createUrl);

        curl_easy_setopt(createHandle, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(createHandle, CURLOPT_POSTFIELDSIZE, static_cast<long>(postFields.length()));

        const CURLcode response = curl_easy_perform(createHandle);
        long responseCode;
        curl_easy_getinfo(createHandle, CURLINFO_RESPONSE_CODE, &responseCode);
        treatCurlResponse(response, responseCode);
        curl_easy_cleanup(createHandle);
    }

} // namespace influxdb
