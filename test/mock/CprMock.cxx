// MIT License
//
// Copyright (c) 2020-2024 offa
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

#include "CprMock.h"
#include <algorithm>

namespace cpr
{
    namespace
    {
        class ParametersSpy : public Parameters
        {
        public:
            static std::map<std::string, std::string> toMap(const Parameters& p)
            {
                const auto& entries = static_cast<const ParametersSpy&>(p).containerList_;
                std::map<std::string, std::string> params{};

                std::transform(entries.cbegin(), entries.cend(), std::inserter(params, params.end()), [](const auto& e)
                               { return std::pair{e.key, e.value}; });
                return params;
            }
        };
    }


    template <class T>
    CurlContainer<T>::CurlContainer(const std::initializer_list<T>& containerList)
        : containerList_(containerList)
    {
    }

    Session::Session() = default;

    void Session::SetTimeout(const Timeout& timeout)
    {
        influxdb::test::sessionMock.SetTimeout(timeout);
    }

    void Session::SetConnectTimeout(const ConnectTimeout& timeout)
    {
        influxdb::test::sessionMock.SetConnectTimeout(timeout);
    }

    void Session::SetBody(Body&& body)
    {
        influxdb::test::sessionMock.SetBody(std::move(body));
    }

    Response Session::Get()
    {
        return influxdb::test::sessionMock.Get();
    }
    Response Session::Post()
    {
        return influxdb::test::sessionMock.Post();
    }

    void Session::SetAuth(const Authentication& auth)
    {
        influxdb::test::sessionMock.SetAuth(auth);
    }

    void Session::SetProxyAuth(ProxyAuthentication&& proxy_auth)
    {
        influxdb::test::sessionMock.SetProxyAuth(std::move(proxy_auth));
    }

    void Session::SetVerifySsl(const VerifySsl& verify)
    {
        influxdb::test::sessionMock.SetVerifySsl(verify);
    }

    void Session::SetParameters(Parameters&& parameters)
    {
        influxdb::test::sessionMock.SetParameters(ParametersSpy::toMap(parameters));
    }

    void Session::SetProxies(Proxies&& proxies)
    {
        influxdb::test::sessionMock.SetProxies(std::move(proxies));
    }

    void Session::SetUrl(const Url& url)
    {
        influxdb::test::sessionMock.SetUrl(url);
    }

    void Session::SetHeader(const Header& header)
    {
        influxdb::test::sessionMock.SetHeader(header);
    }

    void Session::UpdateHeader(const Header& header)
    {
        influxdb::test::sessionMock.UpdateHeader(header);
    }

    Parameters::Parameters(const std::initializer_list<Parameter>& parameters)
        : CurlContainer<Parameter>(parameters)
    {
    }

    Proxies::Proxies(const std::initializer_list<std::pair<const std::string, std::string>>& hosts)
        : hosts_{hosts}
    {
    }

    Authentication::~Authentication() noexcept = default;


    bool CaseInsensitiveCompare::operator()([[maybe_unused]] const std::string& a, [[maybe_unused]] const std::string& b) const noexcept
    {
        return false;
    }

    std::string util::urlEncode(const std::string& s)
    {
        return s;
    }

    CurlHolder::CurlHolder() = default;
    CurlHolder::~CurlHolder() = default;

    const char* Authentication::GetAuthString() const noexcept
    {
        return auth_string_.c_str();
    }

    const std::string& Proxies::operator[](const std::string& protocol)
    {
        if (hosts_.count(protocol) == 0)
        {
            FAIL("Proxies: No entry '" << protocol << "' available");
        }
        return hosts_[protocol];
    }

    const char* ProxyAuthentication::operator[](const std::string& protocol)
    {
        if (proxyAuth_.count(protocol) == 0)
        {
            FAIL("ProxyAuthentication: No entry '" << protocol << "' available");
        }
        return proxyAuth_[protocol].GetAuthString();
    }

    EncodedAuthentication::~EncodedAuthentication() noexcept = default;

    const char* EncodedAuthentication::GetAuthString() const noexcept
    {
        return auth_string_.c_str();
    }

}
