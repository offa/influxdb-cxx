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

#include "HTTP.h"
#include "InfluxDBException.h"
#include "mock/CurlMock.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_version_macros.hpp>
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

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_CONNECTTIMEOUT, long{10})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TIMEOUT, long{10})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPIDLE, long{120})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPINTVL, long{60})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_CONNECTTIMEOUT, long{10})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TIMEOUT, long{10})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPIDLE, long{120})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_TCP_KEEPINTVL, long{60})).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_WRITEFUNCTION, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_URL, "http://localhost:8086/write?db=test")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POST, long{1})).RETURN(CURLE_OK);

        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=test"};
    }

    TEST_CASE("Construction throws if curl init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_FAILED_INIT);

        CHECK_THROWS_AS(HTTP{"http://localhost:8086?db=test"}, InfluxDBException);
    }

    TEST_CASE("Construction throws if curl write handle init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(nullptr);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        CHECK_THROWS_AS(HTTP{"http://localhost:8086?db=test"}, InfluxDBException);
    }

    TEST_CASE("Construction throws if curl read handle init fails", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(CURL_GLOBAL_ALL)).RETURN(CURLE_OK);
        trompeloeil::sequence seq;
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(handle).IN_SEQUENCE(seq);
        REQUIRE_CALL(curlMock, curl_easy_init()).RETURN(nullptr).IN_SEQUENCE(seq);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

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
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        const std::string data{"content-to-send"};
        HTTP http{"http://localhost:8086?db=test"};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POSTFIELDS, data)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_POSTFIELDSIZE, static_cast<long>(data.size()))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        http.send(std::string{data});
    }

    TEST_CASE("Send fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
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
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
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
        ALLOW_CALL(curlMock, curl_free(ptr));
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
        ALLOW_CALL(curlMock, curl_free(_));
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
        ALLOW_CALL(curlMock, curl_free(_));
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
        ALLOW_CALL(curlMock, curl_free(_));
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

    TEST_CASE("Create database configures curl", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=example-to-create"};
        REQUIRE_CALL(curlMock, curl_easy_setopt_(handle, CURLOPT_URL, "http://localhost:8086/query")).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(handle, CURLOPT_POST, long{1})).RETURN(CURLE_OK);

        const std::string data = "q=CREATE DATABASE example-to-create";
        REQUIRE_CALL(curlMock, curl_easy_setopt_(handle, CURLOPT_POSTFIELDS, data)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(handle, CURLOPT_POSTFIELDSIZE, static_cast<long>(data.size()))).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        http.createDatabase();
    }

    TEST_CASE("Create database fails on unsuccessful execution", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=example-to-create"};

        const std::string data = "q=CREATE DATABASE example-to-create";
        REQUIRE_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_FAILED_INIT);
        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);

        REQUIRE_THROWS_AS(http.createDatabase(), ConnectionError);
    }

    TEST_CASE("Create database accepts successful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=example-to-create"};

        const std::string data = "q=CREATE DATABASE example-to-create";
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 200)
            .RETURN(CURLE_OK);
        http.createDatabase();

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 204)
            .RETURN(CURLE_OK);
        http.createDatabase();
    }

    TEST_CASE("Create database throws on unsuccessful response", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=example-to-create"};

        const std::string data = "q=CREATE DATABASE example-to-create";
        ALLOW_CALL(curlMock, curl_easy_perform(handle)).RETURN(CURLE_OK);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 404)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.createDatabase(), NonExistentDatabase);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 400)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.createDatabase(), BadRequest);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 500)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.createDatabase(), ServerError);

        REQUIRE_CALL(curlMock, curl_easy_getinfo_(handle, CURLINFO_RESPONSE_CODE, _))
            .LR_SIDE_EFFECT(*static_cast<long*>(_3) = 503)
            .RETURN(CURLE_OK);
        REQUIRE_THROWS_AS(http.createDatabase(), ServerError);
    }

    TEST_CASE("Enabling basic auth sets curl options", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        HTTP http{"http://localhost:8086?db=example-database-0"};

        REQUIRE_CALL(curlMock, curl_easy_setopt_(handle, CURLOPT_HTTPAUTH, CURLAUTH_BASIC)).RETURN(CURLE_OK).TIMES(2);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(handle, CURLOPT_USERPWD, "user0:pass0")).RETURN(CURLE_OK).TIMES(2);
        http.enableBasicAuth("user0:pass0");
    }

    TEST_CASE("Database name is returned if valid", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        const HTTP http{"http://localhost:8086?db=example-database-0"};
        CHECK(http.databaseName() == "example-database-0");
    }

    TEST_CASE("Database service url is returned if valid", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        const HTTP http{"http://localhost:8086?db=example-database-1"};
        CHECK(http.influxDbServiceUrl() == "http://localhost:8086");
    }

    TEST_CASE("Set proxy without authentication", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_PROXY, "https://proxy-server:1234")).RETURN(CURLE_OK).TIMES(2);

        HTTP http{"http://localhost:8086?db=test"};
        http.setProxy(Proxy{"https://proxy-server:1234"});
    }

    TEST_CASE("Set proxy with authentication", "[HttpTest]")
    {
        ALLOW_CALL(curlMock, curl_global_init(_)).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_init()).RETURN(handle);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(std::string))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(long))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_setopt_(_, _, ANY(WriteCallbackFn))).RETURN(CURLE_OK);
        ALLOW_CALL(curlMock, curl_easy_cleanup(_));
        ALLOW_CALL(curlMock, curl_global_cleanup());

        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_PROXY, "https://proxy-server:1234")).RETURN(CURLE_OK).TIMES(2);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_PROXYUSERNAME, "abc")).RETURN(CURLE_OK).TIMES(2);
        REQUIRE_CALL(curlMock, curl_easy_setopt_(_, CURLOPT_PROXYPASSWORD, "def")).RETURN(CURLE_OK).TIMES(2);

        HTTP http{"http://localhost:8086?db=test"};
        http.setProxy(Proxy{"https://proxy-server:1234", Proxy::Auth{"abc", "def"}});
    }

}
