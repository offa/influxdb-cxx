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
#include "mock/TransportMock.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
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
        CHECK_THROWS_AS(udp->createDatabase(), std::runtime_error);
    }

    TEST_CASE("UDP transport throws on set proxy", "[BoostSupportTest]")
    {
        auto udp = internal::withUdpTransport(http::url{});
        CHECK_THROWS_AS(udp->setProxy(Proxy{"udp://should-throw"}), std::runtime_error);
    }

    TEST_CASE("UDP transport throws on execute query", "[BoostSupportTest]")
    {
        auto udp = internal::withUdpTransport(http::url{});
        CHECK_THROWS_AS(udp->execute("show databases"), std::runtime_error);
    }

    TEST_CASE("Unix socket transport throws on create database", "[BoostSupportTest]")
    {
        auto unix = internal::withUnixSocketTransport(http::url{});
        CHECK_THROWS_AS(unix->createDatabase(), std::runtime_error);
    }

    TEST_CASE("Unix socket transport throws on set proxy", "[BoostSupportTest]")
    {
        auto unix = internal::withUnixSocketTransport(http::url{});
        CHECK_THROWS_AS(unix->setProxy(Proxy{"unix:///tmp/should_throw"}), std::runtime_error);
    }

    TEST_CASE("Unix socket transport throws on execute query", "[BoostSupportTest]")
    {
        auto unix = internal::withUnixSocketTransport(http::url{});
        CHECK_THROWS_AS(unix->execute("show databases"), std::runtime_error);
    }

    TEST_CASE("Query passes to transport and returns result", "[BoostSupportTest]")
    {
        TransportMock transport;
        REQUIRE_CALL(transport, query("SELECT * from test WHERE host = 'localhost'")).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0
    }
  ]
}
)");

        const auto result = internal::queryImpl(&transport, "SELECT * from test WHERE host = 'localhost'");
        CHECK(result.empty());
    }

    TEST_CASE("Query throws InfluxDBException when transport throws", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).THROW(InfluxDBException{"Intentional"});

        CHECK_THROWS_AS(internal::queryImpl(&transport, "select should throw"), InfluxDBException);
    }

    TEST_CASE("Query returns empty result when response has empty results array", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": []
}
)");

        CHECK(internal::queryImpl(&transport, "SELECT * from test").empty());
    }

    TEST_CASE("Query returns point for single result", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "unittest",
          "tags": {"host": "localhost"},
          "columns": ["time", "value"],
          "values": [["2021-01-01T00:11:22.123456789Z", 112233]]
        }
      ]
    }
  ]
}
)");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        REQUIRE(result.size() == 1);
        const auto point = result[0];
        CHECK(point.getName() == "unittest");
        CHECK(std::format("{:%FT%T}Z", point.getTimestamp()) == "2021-01-01T00:11:22.123456789Z");
        CHECK(point.getTags() == "host=localhost");
        CHECK(point.getFields() == "value=112233.000000000000000000");
    }

    TEST_CASE("Query returns multiple points for multiple results", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "unittest",
          "tags": {"host": "host-0"},
          "columns": ["time", "value"],
          "values": [["2021-01-01T00:11:22.000000000Z", 100]]
        },
        {
          "name": "unittest",
          "tags": {"host": "host-1"},
          "columns": ["time", "value"],
          "values": [["2021-01-01T00:11:23.560000000Z", 30]]
        },
        {
          "name": "unittest",
          "tags": {"host": "host-2"},
          "columns": ["time", "value"],
          "values": [["2021-01-01T00:11:24.780000000Z", 54]]
        }
      ]
    }
  ]
}
)");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        REQUIRE(result.size() == 3);
        CHECK(result[0].getName() == "unittest");
        CHECK(result[0].getTags() == "host=host-0");
        CHECK(result[0].getFields() == "value=100.000000000000000000");
        CHECK(result[1].getName() == "unittest");
        CHECK(result[1].getTags() == "host=host-1");
        CHECK(result[1].getFields() == "value=30.000000000000000000");
        CHECK(result[2].getName() == "unittest");
        CHECK(result[2].getTags() == "host=host-2");
        CHECK(result[2].getFields() == "value=54.000000000000000000");
    }

    TEST_CASE("Query throws on invalid JSON response", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
invalid-results ":x"]})");

        CHECK_THROWS(internal::queryImpl(&transport, "SELECT * from test"));
    }

    TEST_CASE("Query handles empty measurement name", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "tags": {"host": "x"},
          "columns": ["time", "value"],
          "values": [["2021-01-01T00:11:22.000000000Z", 8]]
        }
      ]
    }
  ]
}
)");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        CHECK(result[0].getName() == "");
        CHECK(result[0].getTags() == "host=x");
    }

    TEST_CASE("Query reads optional tags element", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "x",
          "tags": {"type": "sp"},
          "columns": ["time", "value"],
          "values": [["2022-01-01T00:01:02.000000000Z", 99]]
        }
      ]
    }
  ]
}
)");

        const auto result = internal::queryImpl(&transport, "SELECT * from test");
        CHECK(result.size() == 1);
        CHECK(result[0].getTags() == "type=sp");
    }

    TEST_CASE("Query handles valid response with string values", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "cpu",
          "columns": ["time", "value"],
          "values": [["2023-01-01T00:00:00Z", "high"]]
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM cpu");
        REQUIRE(result.size() == 1);
        CHECK(result[0].getFields() == "value=\"high\"");
    }

    TEST_CASE("Query handles valid response with boolean values", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "status",
          "columns": ["time", "active"],
          "values": [["2023-01-01T00:00:00Z", true]]
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM status");
        REQUIRE(result.size() == 1);
        CHECK(result[0].getFields() == "active=true");
    }

    TEST_CASE("Query handles valid response with null values", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "measurements",
          "columns": ["time", "value"],
          "values": [["2023-01-01T00:00:00Z", null]]
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM measurements");
        REQUIRE(result.size() == 1);
        const auto& point = result[0];
        REQUIRE(point.getName() == "measurements");
        REQUIRE(point.getTagSet().empty());
        REQUIRE(point.getFieldSet().empty());
    }

    TEST_CASE("Query handles response with missing series name", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "columns": ["time", "value"],
          "values": [["2023-01-01T00:00:00Z", 1221]]
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM test");
        REQUIRE(result.size() == 1);
        const auto& point = result[0];
        REQUIRE(point.getName() == "");
        REQUIRE(point.getFieldSet().size() == 1);
        REQUIRE(point.getTagSet().empty());
    }

    TEST_CASE("Query handles response with multiple series", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "cpu",
          "columns": ["time", "value"],
          "values": [["2023-01-01T00:00:00Z", 80]]
        },
        {
          "name": "memory",
          "columns": ["time", "value"],
          "values": [["2023-01-01T00:00:00Z", 90]]
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM system");
        REQUIRE(result.size() == 2);
        CHECK(result[0].getFields() == "value=80.000000000000000000");
        CHECK(result[1].getFields() == "value=90.000000000000000000");
    }

    TEST_CASE("Query handles response with empty columns array", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "test",
          "columns": [],
          "values": []
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM test");
        REQUIRE(result.size() == 0);
    }

    TEST_CASE("Query handles mismatched column and value counts", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "test",
          "columns": ["time", "value1", "value2"],
          "values": [["2023-01-01T00:00:00Z", 123123]]
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM test");
        REQUIRE(result.size() == 1);
        const auto& point = result[0];
        REQUIRE(point.getName() == "test");

        const auto& fields = point.getFieldSet();
        REQUIRE(fields.size() == 1);

        const auto itr = std::find_if(fields.cbegin(), fields.cend(),
                                      [](const auto& field)
                                      { return field.first == "value1"; });
        REQUIRE(itr != fields.end());
        REQUIRE_THAT(std::get<double>(itr->second), Catch::Matchers::WithinAbs(123123.0, 1e-6));

        REQUIRE(std::none_of(fields.cbegin(), fields.cend(),
                             [](const auto& field)
                             { return field.first == "value2"; }));

        REQUIRE(point.getTagSet().empty());
    }

    TEST_CASE("Query handles non-string column names", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "test",
          "columns": ["time", 123],
          "values": [["2023-01-01T00:00:00Z", 567999]]
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM test");
        REQUIRE(result.size() == 1);
        const auto& point = result[0];
        REQUIRE(point.getName() == "test");
        REQUIRE(point.getFieldSet().empty());
        REQUIRE(point.getTagSet().empty());
    }

    TEST_CASE("Query handles valid response with timestamp in different formats", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": "test",
          "columns": ["time", "value"],
          "values": [["2023-01-01T00:00:00Z", 9999]]
        }
      ]
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM test");
        REQUIRE(result.size() == 1);
    }

    TEST_CASE("Query throws on JSON parsing error", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        {
          "name": this is invalid JSON
          "columns": ["time", "value"]
        }
      ]
    }
  ]
}
)");

        CHECK_THROWS(internal::queryImpl(&transport, "SELECT * FROM test"));
    }

    TEST_CASE("Query throws on InfluxDB error response", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "error": "database not found"
}
)");

        CHECK_THROWS_AS(influxdb::internal::queryImpl(&transport, "SELECT * FROM nonexistent"), std::runtime_error);
    }

    TEST_CASE("Query returns empty result for response with empty series array", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": []
    }
  ]
}
)");

        const auto result = influxdb::internal::queryImpl(&transport, "SELECT * FROM test");
        REQUIRE(result.size() == 0);
    }

    TEST_CASE("Query throws on response with non-object series", "[BoostSupportTest]")
    {
        using trompeloeil::_;

        TransportMock transport;
        ALLOW_CALL(transport, query(_)).RETURN(R"(
{
  "results": [
    {
      "statement_id": 0,
      "series": [
        "invalid_series"
      ]
    }
  ]
}
)");

        CHECK_THROWS_AS(influxdb::internal::queryImpl(&transport, "SELECT * FROM test"), std::runtime_error);
    }
}
