// MIT License
//
// Copyright (c) 2020-2023 offa
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
#include "InfluxDBException.h"
#include <catch2/catch_test_macros.hpp>

namespace influxdb::test
{
    namespace
    {
        struct TransportDummy : public Transport
        {
            void send([[maybe_unused]] std::string&& message) override
            {
            }

            std::size_t getMaxMessageSize() const override
            {
                return (std::numeric_limits<std::size_t>::max)();
            }
        };

        TransportDummy dummy;
    }


    TEST_CASE("Query impl throws unconditionally", "[NoBoostSupportTest]")
    {
        CHECK_THROWS_AS(internal::queryImpl(&dummy, "-ignore-"), InfluxDBException);
    }

    TEST_CASE("With UDP throws transport unconditionally", "[NoBoostSupportTest]")
    {
        CHECK_THROWS_AS(internal::withUdpTransport(http::url{}), InfluxDBException);
    }

    TEST_CASE("With Unix socket transport throws unconditionally", "[NoBoostSupportTest]")
    {
        CHECK_THROWS_AS(internal::withUnixSocketTransport(http::url{}), InfluxDBException);
    }

}
