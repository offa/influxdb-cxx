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

#include "BoostSupport.h"
#include "InfluxDB/InfluxDBException.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    TEST_CASE("UDP transport returns valid transport", "[BoostSupportTest]")
    {
        CHECK(internal::withUdpTransport(http::url{}) != nullptr);
    }

    TEST_CASE("Unix socket transport returns valid transport", "[BoostSupportTest]")
    {
        CHECK(internal::withUnixSocketTransport(http::url{}) != nullptr);
    }

    TEST_CASE("UDP transport throws on create database", "[BoostSupportTest]")
    {
        auto udp = internal::withUdpTransport(http::url{});
        CHECK_THROWS_AS(udp->createDatabase(), InfluxDBException);
    }

    TEST_CASE("UDP transport throws on set proxy", "[BoostSupportTest]")
    {
        auto udp = internal::withUdpTransport(http::url{});
        CHECK_THROWS_AS(udp->setProxy(Proxy{"udp://should-throw"}), InfluxDBException);
    }

    TEST_CASE("UDP transport throws on execute query", "[BoostSupportTest]")
    {
        auto udp = internal::withUdpTransport(http::url{});
        CHECK_THROWS_AS(udp->execute("show databases"), InfluxDBException);
    }

    TEST_CASE("Unix socket transport throws on create database", "[BoostSupportTest]")
    {
        auto unix = internal::withUnixSocketTransport(http::url{});
        CHECK_THROWS_AS(unix->createDatabase(), InfluxDBException);
    }

    TEST_CASE("Unix socket transport throws on set proxy", "[BoostSupportTest]")
    {
        auto unix = internal::withUnixSocketTransport(http::url{});
        CHECK_THROWS_AS(unix->setProxy(Proxy{"unix:///tmp/should_throw"}), InfluxDBException);
    }

    TEST_CASE("Unix socket transport throws on execute query", "[BoostSupportTest]")
    {
        auto unix = internal::withUnixSocketTransport(http::url{});
        CHECK_THROWS_AS(unix->execute("show databases"), InfluxDBException);
    }

}
