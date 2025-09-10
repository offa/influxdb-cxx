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

#pragma once

#include "InfluxDB/InfluxDBFactory.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <string>
#include <optional>
#include <cstdlib>

namespace influxdb::test
{
    struct User
    {
        std::string name;
        std::string pass;
    };


    inline std::optional<std::string> getEnv(const std::string& name)
    {
        if (const auto value = std::getenv(name.c_str()); value != nullptr)
        {
            return value;
        }
        return {};
    }

    inline User getUserFromEnv()
    {
        const auto user = getEnv("INFLUXDBCXX_SYSTEMTEST_USER");
        const auto pass = getEnv("INFLUXDBCXX_SYSTEMTEST_PASSWORD");

        if (!user || !pass)
        {
            SKIP("No authentication configured: 'INFLUXDBCXX_SYSTEMTEST_USER' and/or 'INFLUXDBCXX_SYSTEMTEST_PASSWORD' not set");
            return {"", ""};
        }
        return {*user, *pass};
    }

    inline std::string getHostFromEnv()
    {
        return getEnv("INFLUXDBCXX_SYSTEMTEST_HOST").value_or("localhost");
    }

    inline std::unique_ptr<InfluxDB> configure(const std::string& db, std::optional<User> user = {})
    {
        const std::string authString{user ? (user->name + ":" + user->pass + "@") : ""};
        return InfluxDBFactory::Get("http://" + authString + getHostFromEnv() + ":8086?db=" + db);
    }
}
