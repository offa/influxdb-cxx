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

#pragma once

#include <cpr/cpr.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>
#include <map>

namespace influxdb::test
{

    class SessionMock
    {
    public:
        MAKE_MOCK1(SetTimeout, void(const cpr::Timeout&));
        MAKE_MOCK1(SetConnectTimeout, void(const cpr::ConnectTimeout&));
        MAKE_MOCK0(Get, cpr::Response());
        MAKE_MOCK0(Post, cpr::Response());
        MAKE_MOCK1(SetUrl, void(const cpr::Url&));
        MAKE_MOCK1(SetHeader, void(const cpr::Header&));
        MAKE_MOCK1(UpdateHeader, void(const cpr::Header&));
        MAKE_MOCK1(SetBody, void(cpr::Body&&));
        MAKE_MOCK1(SetParameters, void(std::map<std::string, std::string>));
        MAKE_MOCK1(SetAuth, void(const cpr::Authentication&));
        MAKE_MOCK1(SetProxies, void(cpr::Proxies&&));
        MAKE_MOCK1(SetProxyAuth, void(cpr::ProxyAuthentication&&));
        MAKE_MOCK1(SetVerifySsl, void(const cpr::VerifySsl&));
    };

    extern SessionMock sessionMock;
}
