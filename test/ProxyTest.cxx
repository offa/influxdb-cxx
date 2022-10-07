// MIT License
//
// Copyright (c) 2020-2022 offa
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

#include "Proxy.h"
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/catch_test_macros.hpp>


namespace influxdb::test
{
    using namespace Catch::Matchers;

    TEST_CASE("Proxy without authentication", "[ProxyTest]")
    {
        const Proxy p("https://proxy-host");
        CHECK_THAT(p.getProxy(), Equals("https://proxy-host"));
        CHECK(p.getAuthentication().has_value() == false);
    }

    TEST_CASE("Proxy with authentication", "[ProxyTest]")
    {
        const Proxy p("https://proxy-host", Proxy::Auth{"uname", "upw"});
        CHECK_THAT(p.getProxy(), Equals("https://proxy-host"));

        const auto auth = p.getAuthentication();
        CHECK(auth.has_value() == true);
        CHECK_THAT(auth->user, Equals("uname"));
        CHECK_THAT(auth->password, Equals("upw"));
    }

}
