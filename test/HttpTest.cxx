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

#include "HTTP.h"
#include "InfluxDB/InfluxDBException.h"
#include "mock/CprMock.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    SessionMock sessionMock;

    using influxdb::transports::HTTP;
    using trompeloeil::_;
    using trompeloeil::eq;

    using ParamMap = std::map<std::string, std::string>;

    cpr::Response createResponse(const cpr::ErrorCode& code, std::int32_t statusCode, const std::string& text = "<text placeholder>")
    {
        cpr::Error error{};
        error.code = code;
        error.message = "<error message placeholder";

        cpr::Response response{};
        response.error = error;
        response.status_code = statusCode;
        response.text = text;
        return response;
    }

    HTTP createHttp()
    {
        ALLOW_CALL(sessionMock, SetTimeout(_));
        ALLOW_CALL(sessionMock, SetConnectTimeout(_));
        return HTTP{"http://localhost:8086?db=test"};
    }


    TEST_CASE("Construction fails on missing url part", "[HttpTest]")
    {
        REQUIRE_THROWS_AS(HTTP{"http://localhost:8086"}, InfluxDBException);
        REQUIRE_THROWS_AS(HTTP{"http://localhost:8086?"}, InfluxDBException);
        REQUIRE_THROWS_AS(HTTP{"http://localhost:8086!db=test"}, InfluxDBException);
        REQUIRE_THROWS_AS(HTTP{"http://localhost:8086?dc=test"}, InfluxDBException);
        REQUIRE_THROWS_AS(HTTP{"http://localhost:8086?dbtest"}, InfluxDBException);
        REQUIRE_THROWS_AS(HTTP{"http://localhost:8086?db-test"}, InfluxDBException);
    }

    TEST_CASE("Construction sets session settings", "[HttpTest]")
    {
        REQUIRE_CALL(sessionMock, SetTimeout(_));
        REQUIRE_CALL(sessionMock, SetConnectTimeout(_));

        HTTP http{"http://localhost:8086?db=test"};
    }

    TEST_CASE("Send sets parameters", "[HttpTest]")
    {
        auto http = createHttp();
        const std::string data{"content-to-send"};

        REQUIRE_CALL(sessionMock, Post()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_OK));
        REQUIRE_CALL(sessionMock, SetUrl(eq("http://localhost:8086/write")));
        REQUIRE_CALL(sessionMock, UpdateHeader(_)).WITH(_1.at("Content-Type") == "application/json");
        REQUIRE_CALL(sessionMock, SetBody(_)).WITH(_1.str() == data);
        REQUIRE_CALL(sessionMock, SetParameters(ParamMap{{"db", "test"}}));

        http.send(std::string{data});
    }

    TEST_CASE("Send fails on unsuccessful execution", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Post()).RETURN(createResponse(cpr::ErrorCode::SEND_ERROR, cpr::status::HTTP_OK));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, UpdateHeader(_));
        ALLOW_CALL(sessionMock, SetBody(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        REQUIRE_THROWS_AS(http.send("content"), InfluxDBException);
    }

    TEST_CASE("Send accepts successful response", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Post()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_OK));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, UpdateHeader(_));
        ALLOW_CALL(sessionMock, SetBody(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        http.send("content");
    }

    TEST_CASE("Send throws on unsuccessful response", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Post()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_NOT_FOUND));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, UpdateHeader(_));
        ALLOW_CALL(sessionMock, SetBody(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        REQUIRE_THROWS_AS(http.send("content"), InfluxDBException);
    }

    TEST_CASE("Query sets parameters", "[HttpTest]")
    {
        auto http = createHttp();
        const std::string query{"/12?ab=cd"};

        REQUIRE_CALL(sessionMock, Get()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_OK, "query-result"));
        REQUIRE_CALL(sessionMock, SetUrl(eq("http://localhost:8086/query")));
        REQUIRE_CALL(sessionMock, SetParameters(ParamMap{{"db", "test"}, {"q", query}}));

        CHECK(http.query(query) == "query-result");
    }

    TEST_CASE("Query fails on unsuccessful execution", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Get()).RETURN(createResponse(cpr::ErrorCode::COULDNT_CONNECT, cpr::status::HTTP_OK));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        REQUIRE_THROWS_AS(http.query("/12?ab=cd"), InfluxDBException);
    }

    TEST_CASE("Query accepts successful response", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Get()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_OK, "query-result"));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        CHECK(http.query("/12?ab=cd") == "query-result");
    }

    TEST_CASE("Query throws on unsuccessful response", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Get()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_BAD_GATEWAY));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        REQUIRE_THROWS_AS(http.query("/12?ab=cd"), InfluxDBException);
    }

    TEST_CASE("Create database sets parameters", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Post()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_OK));
        REQUIRE_CALL(sessionMock, SetUrl(eq("http://localhost:8086/query")));
        REQUIRE_CALL(sessionMock, SetParameters(ParamMap{{"q", "CREATE DATABASE test"}}));

        http.createDatabase();
    }

    TEST_CASE("Create database fails on unsuccessful execution", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Post()).RETURN(createResponse(cpr::ErrorCode::UNKNOWN_ERROR, cpr::status::HTTP_OK));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        REQUIRE_THROWS_AS(http.createDatabase(), InfluxDBException);
    }

    TEST_CASE("Create database accepts successful response", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Post()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_OK));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        http.createDatabase();
    }

    TEST_CASE("Create database throws on unsuccessful response", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Post()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_BAD_GATEWAY));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        REQUIRE_THROWS_AS(http.createDatabase(), InfluxDBException);
    }

    TEST_CASE("Set authentication sets parameters", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, SetAuth(_)).WITH(_1.GetAuthString() == std::string{"user0:pass0"});
        http.setBasicAuthentication("user0", "pass0");
    }

    TEST_CASE("Set auth token sets auth header", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, UpdateHeader(_)).WITH(_1.at("Authorization") == "Token not-a-real-api-token");
        http.setAuthToken("not-a-real-api-token");
    }

    TEST_CASE("Set proxy without authentication", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, SetProxies(_)).WITH(_1["http"] == std::string{"https://proxy-server:1234"} && _1["https"] == std::string{"https://proxy-server:1234"});
        http.setProxy(Proxy{"https://proxy-server:1234"});
    }

    TEST_CASE("Set proxy with authentication", "[HttpTest]")
    {
        using namespace std::string_literals;
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, SetProxies(_)).WITH(_1["http"] == std::string{"https://auth-proxy-server:1234"} && _1["https"] == std::string{"https://auth-proxy-server:1234"});
        REQUIRE_CALL(sessionMock, SetProxyAuth(_)).WITH(_1.GetUsername("http") == "abc"s && _1.GetPassword("http") == "def"s && _1.GetUsername("https") == "abc"s && _1.GetPassword("https") == "def"s);

        http.setProxy(Proxy{"https://auth-proxy-server:1234", Proxy::Auth{"abc", "def"}});
    }

    TEST_CASE("Set certificate verification", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, SetVerifySsl(_)).WITH(bool{_1} == false);
        http.setVerifyCertificate(false);

        REQUIRE_CALL(sessionMock, SetVerifySsl(_)).WITH(bool{_1} == true);
        http.setVerifyCertificate(true);
    }

    TEST_CASE("Set timeout sets timeouts", "[HttpTest]")
    {
        constexpr std::chrono::seconds timeout{3};
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, SetTimeout(_)).WITH(_1.ms == timeout);
        REQUIRE_CALL(sessionMock, SetConnectTimeout(_)).WITH(_1.ms == timeout);
        http.setTimeout(timeout);
    }

    TEST_CASE("Execute sets parameters", "[HttpTest]")
    {
        auto http = createHttp();
        const std::string cmd{"show databases"};

        REQUIRE_CALL(sessionMock, Get()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_OK, "response-of-execute"));
        REQUIRE_CALL(sessionMock, SetUrl(eq("http://localhost:8086/query")));
        REQUIRE_CALL(sessionMock, SetParameters(ParamMap{{"db", "test"}, {"q", cmd}}));

        CHECK(http.execute(cmd) == "response-of-execute");
    }

    TEST_CASE("Execute fails on unsuccessful execution", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Get()).RETURN(createResponse(cpr::ErrorCode::COULDNT_CONNECT, cpr::status::HTTP_OK));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        REQUIRE_THROWS_AS(http.execute("fail-execution"), InfluxDBException);
    }

    TEST_CASE("Execute accepts successful response", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Get()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_OK, "response-of-execute"));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        CHECK(http.execute("show databases") == "response-of-execute");
    }

    TEST_CASE("Execute throws on unsuccessful response", "[HttpTest]")
    {
        auto http = createHttp();

        REQUIRE_CALL(sessionMock, Get()).RETURN(createResponse(cpr::ErrorCode::OK, cpr::status::HTTP_NOT_FOUND));
        ALLOW_CALL(sessionMock, SetUrl(_));
        ALLOW_CALL(sessionMock, SetParameters(_));

        REQUIRE_THROWS_AS(http.execute("fail-execution"), InfluxDBException);
    }

}
