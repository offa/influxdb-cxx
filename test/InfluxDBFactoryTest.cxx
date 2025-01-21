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

#include "InfluxDB/InfluxDBFactory.h"
#include "InfluxDB/InfluxDBException.h"
#include <catch2/catch_test_macros.hpp>

namespace influxdb::test
{
    TEST_CASE("Accepts http urls", "[InfluxDBFactoryTest]")
    {
        CHECK(InfluxDBFactory::Get("http://localhost?db=test") != nullptr);
        CHECK(InfluxDBFactory::Get("https://localhost?db=test") != nullptr);

        CHECK(InfluxDBFactory::Get("http://localhost:8086?db=test") != nullptr);
        CHECK(InfluxDBFactory::Get("https://localhost:8086?db=test") != nullptr);

        CHECK(InfluxDBFactory::Get("https://localhost/?db=test") != nullptr);
        CHECK(InfluxDBFactory::Get("https://localhost:8086/?db=test") != nullptr);
    }

    TEST_CASE("Accepts http urls with authentication", "[InfluxDBFactoryTest]")
    {
        CHECK(InfluxDBFactory::Get("http://user:pass@localhost?db=test") != nullptr);
    }

    TEST_CASE("Throws on unrecognised backend", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::Get("httpX://localhost:8086?db=test"), InfluxDBException);
    }

    TEST_CASE("Throws on malformed url", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::Get("localhost:8086?db=test"), InfluxDBException);
    }

    TEST_CASE("Throws on missing database", "[InfluxDBFactoryTest]")
    {
        CHECK_THROWS_AS(InfluxDBFactory::Get("http://localhost:8086"), InfluxDBException);
    }
}
