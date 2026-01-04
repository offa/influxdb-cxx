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

#include "SystemTest.h"
#include "InfluxDB/InfluxDBBuilder.h"

namespace influxdb::test
{
    TEST_CASE("Http authentication test", "[HttpAuthTest]")
    {
        using Catch::Matchers::ContainsSubstring;

        SECTION("Unauthenticated users fails")
        {
            auto db = configure("st_auth_db", {});
            CHECK_THROWS_AS(db->execute("show users"), InfluxDBException);
        }

        SECTION("Authenticated user has access")
        {
            const auto user = getUserFromEnv();
            auto db = configure("st_auth_db", user);

            const auto response = db->execute("show users");
            CHECK_THAT(response, ContainsSubstring(user.name));
        }
    }

    TEST_CASE("Http authentication methods", "[HttpAuthTest]")
    {
        using Catch::Matchers::ContainsSubstring;

        const auto user = getUserFromEnv();

        SECTION("Authenticate through factory")
        {
            auto db = InfluxDBFactory::Get("http://" + (user.name + ":" + user.pass + "@") + getHostFromEnv() + ":8086?db=ignore");

            const auto response = db->execute("show users");
            CHECK_THAT(response, ContainsSubstring(user.name));
        }

        SECTION("Authenticate through builder")
        {
            auto db = InfluxDBBuilder::http("http://" + getHostFromEnv() + ":8086?db=ignore")
                          .setBasicAuthentication(user.name, user.pass)
                          .connect();

            const auto response = db->execute("show users");
            CHECK_THAT(response, ContainsSubstring(user.name));
        }
    }
}
