// MIT License
//
// Copyright (c) 2020 offa
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

#include "HTTP.h"
#include "InfluxDBException.h"
#include "mock/CurlMock.h"
#include <catch2/catch.hpp>
#include <catch2/trompeloeil.hpp>


namespace influxdb::test
{
    namespace
    {
        CurlHandleDummy dummy;
        CURL* handle = &dummy;
    }

    CurlMock curlMock;

    using influxdb::transports::HTTP;
    using trompeloeil::_;

    TEST_CASE("Construction initializes curl", "[HttpTest]")
    {
        REQUIRE_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle);

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_CONNECTTIMEOUT, 10)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TIMEOUT, 10)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPIDLE, 120)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPINTVL, 60)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_CONNECTTIMEOUT, 10)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TIMEOUT, 10)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPIDLE, 120)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPINTVL, 60)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/write?db=test")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POST)).RETURN(CURLE_OK);

        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};
    }

    TEST_CASE("Construction throws if curl init fails", "[HttpTest]")
    {
        REQUIRE_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_FAILED_INIT);

        CHECK_THROWS_AS(HTTP{"http://localhost:8086?db=test"}, InfluxDBException);
    }

    TEST_CASE("Construction throws if no database parameter in url", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);

        CHECK_THROWS_AS(HTTP{"http://localhost:8086-db=test"}, InfluxDBException);
    }

    TEST_CASE("Destruction cleans up curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);

        {
            REQUIRE_CALL(curlMock, curl_easy_cleanup(handle)).TIMES(2);
            REQUIRE_CALL(curlMock, curl_global_cleanup());

            HTTP http{"http://localhost:8086?db=test"};
        }
    }

    TEST_CASE("Send configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POSTFIELDS)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POSTFIELDSIZE)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        http.send("content");
    }

    TEST_CASE("Send fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};

        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_FAILED_INIT);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 99)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.send("content"), ConnectionError);
    }

    TEST_CASE("Send accepts successful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};

        ALLOW_CALL(curlMock, curl_easy_perform(_)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);
        http.send("content");

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 204)
            .RETURN(CURLE_OK);
        http.send("content");
    }

    TEST_CASE("Send throws on unsuccessful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};

        ALLOW_CALL(curlMock, curl_easy_perform(_)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 404)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), NonExistentDatabase);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), BadRequest);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 500)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), ServerError);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 503)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.send("content"), ServerError);
    }

    TEST_CASE("Query configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};

        const std::string query{"/12?ab=cd"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        REQUIRE_CALL(curlMock, curl_easy_escape(handle, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/query?db=test&q=/12?ab=cd")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        const auto result = http.query(query);
        CHECK(result == "query-result");
    }

    TEST_CASE("Query fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(handle, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_FAILED_INIT);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 99)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.query(query), ConnectionError);
    }

    TEST_CASE("Query accepts successful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(handle, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);
        http.query(query);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 204)
            .RETURN(CURLE_OK);

        const auto result = http.query(query);
        CHECK(result == "query-result");
    }

    TEST_CASE("Query throws on unsuccessful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};

        const std::string query{"/x?shouldfail=true"};
        std::string returnValue = query;
        char* ptr = &returnValue[0];
        ALLOW_CALL(curlMock, curl_easy_escape(handle, query.c_str(), static_cast<int>(query.size()))).RETURN(ptr);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEDATA, ANY(void*)))
            .LR_SIDE_EFFECT(*static_cast<std::string*>(_3) = "query-result")
            .RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 404)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), NonExistentDatabase);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), BadRequest);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 500)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), ServerError);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 503)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.query(query), ServerError);
    }
}
