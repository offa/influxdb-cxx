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

#include "UDP.h"
#include "InfluxDBException.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    using influxdb::transports::UDP;
    using Catch::Matchers::ContainsSubstring;

    constexpr int DEFAULT_UDP_PORT{8089};

    UDP createUDP()
    {
        return UDP{"localhost", DEFAULT_UDP_PORT};
    }

    TEST_CASE("Construction succeeds with resolvable host", "[UDPTest]")
    {
        REQUIRE_NOTHROW(createUDP());
    }

    TEST_CASE("Construction fails on name resolution error", "[UDPTest]")
    {
        // RFC2606 ".invalid" TLD should not resolve
        REQUIRE_THROWS_AS(UDP("hostname.invalid", DEFAULT_UDP_PORT), InfluxDBException);
    }
}
