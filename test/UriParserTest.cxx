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

#include "UriParser.h"
#include "InfluxDB/InfluxDBException.h"
#include <string>
#include <string_view>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

namespace influxdb::test
{
    using Catch::Matchers::ContainsSubstring;
    using internal::AuthenticationType;
    using internal::Endpoint;
    using internal::Scheme;

    namespace
    {
        Endpoint parse(std::string_view uri)
        {
            return internal::parseEndpoint(uri);
        }

        struct RejectionCase
        {
            std::string_view input;
            std::string_view reason;
        };

        void checkRejected(const RejectionCase& testCase)
        {
            CAPTURE(testCase.input);
            CHECK_THROWS_WITH(parse(testCase.input),
                              ContainsSubstring(std::string{testCase.reason}));
        }

        [[nodiscard]] std::string getExceptionMessage(std::string_view input)
        {
            try
            {
                parse(input);
            }
            catch (const InfluxDBException& exception)
            {
                return exception.what();
            }

            FAIL("Expected InfluxDBException");
            return {};
        }
    }

    TEST_CASE("Parses HTTP and HTTPS endpoints", "[UriParserTest]")
    {
        SECTION("HTTP uses 8086 by default")
        {
            const auto endpoint = parse("http://localhost");
            CHECK(endpoint.scheme == Scheme::Http);
            CHECK(endpoint.host == "localhost");
            CHECK(endpoint.port == 8086);
            CHECK(endpoint.path.empty());
            CHECK(endpoint.authentication.type == AuthenticationType::None);
        }

        SECTION("HTTPS uses 8086 by default")
        {
            const auto endpoint = parse("https://localhost");
            CHECK(endpoint.scheme == Scheme::Https);
            CHECK(endpoint.host == "localhost");
            CHECK(endpoint.port == 8086);
            CHECK(endpoint.path.empty());
        }

        SECTION("Explicit ports override defaults")
        {
            CHECK(parse("http://localhost:80").port == 80);
            CHECK(parse("http://localhost:8086").port == 8086);
            CHECK(parse("https://localhost:443").port == 443);
            CHECK(parse("https://localhost:8443").port == 8443);
        }

        SECTION("Path is preserved and query is excluded")
        {
            const auto endpoint = parse("https://example.com:8443/base/path?db=test");
            CHECK(endpoint.scheme == Scheme::Https);
            CHECK(endpoint.host == "example.com");
            CHECK(endpoint.port == 8443);
            CHECK(endpoint.path == "/base/path");

            CHECK(parse("http://localhost:8086?db=test").path.empty());
            CHECK(parse("http://localhost:8086/").path == "/");
        }
    }

    TEST_CASE("Parses TCP and UDP endpoints", "[UriParserTest]")
    {
        SECTION("TCP")
        {
            const auto endpoint = parse("tcp://localhost:8094");
            CHECK(endpoint.scheme == Scheme::Tcp);
            CHECK(endpoint.host == "localhost");
            CHECK(endpoint.port == 8094);
            CHECK(endpoint.path.empty());
            CHECK(endpoint.authentication.type == AuthenticationType::None);
        }

        SECTION("UDP")
        {
            const auto endpoint = parse("udp://localhost:8089");
            CHECK(endpoint.scheme == Scheme::Udp);
            CHECK(endpoint.host == "localhost");
            CHECK(endpoint.port == 8089);
            CHECK(endpoint.path.empty());
        }

        SECTION("IPv4 hosts")
        {
            CHECK(parse("tcp://192.168.1.10:8094").host == "192.168.1.10");
            CHECK(parse("udp://192.168.1.10:8089").host == "192.168.1.10");
        }
    }

    TEST_CASE("Parses Unix socket endpoints", "[UriParserTest]")
    {
        const auto endpoint = parse("unix:///var/run/influxdb.sock");
        CHECK(endpoint.scheme == Scheme::Unix);
        CHECK(endpoint.host.empty());
        CHECK(endpoint.port == 0);
        CHECK(endpoint.path == "/var/run/influxdb.sock");
        CHECK(endpoint.authentication.type == AuthenticationType::None);

        const auto withAt = parse("unix:///tmp/socket@name");
        CHECK(withAt.path == "/tmp/socket@name");
        CHECK(withAt.authentication.type == AuthenticationType::None);

        const auto withColon = parse("unix:///tmp/user:pass.sock");
        CHECK(withColon.path == "/tmp/user:pass.sock");
        CHECK(withColon.authentication.type == AuthenticationType::None);
    }

    TEST_CASE("Parses DNS, IPv4 and IPv6 hosts", "[UriParserTest]")
    {
        CHECK(parse("http://example.com:8086").host == "example.com");
        CHECK(parse("http://influxdb.local:8086").host == "influxdb.local");
        CHECK(parse("http://192.168.1.10:8086").host == "192.168.1.10");
        CHECK(parse("http://127.0.0.1:8086").host == "127.0.0.1");

        const auto ipv6 = parse("http://[::1]:8086");
        CHECK(ipv6.host == "::1");
        CHECK(ipv6.port == 8086);

        const auto ipv6DefaultPort = parse("https://[2001:db8::1]");
        CHECK(ipv6DefaultPort.host == "2001:db8::1");
        CHECK(ipv6DefaultPort.port == 8086);

        CHECK(parse("https://[2001:db8::1]:8443").port == 8443);
        CHECK(parse("tcp://[::1]:8094").host == "::1");
        CHECK(parse("udp://[::1]:8089").host == "::1");
    }

    TEST_CASE("No authentication", "[UriParserTest]")
    {
        const auto input = GENERATE(
            std::string_view{"http://localhost:8086"},
            std::string_view{"http://[::1]:8086"},
            std::string_view{"tcp://localhost:8094"});

        CAPTURE(input);

        const auto endpoint = parse(input);
        CHECK(endpoint.authentication.type == AuthenticationType::None);
        CHECK(endpoint.authentication.username.empty());
        CHECK(endpoint.authentication.password.empty());
        CHECK(endpoint.authentication.apiKey.empty());
    }

    TEST_CASE("Parses Basic authentication", "[UriParserTest]")
    {
        SECTION("DNS host")
        {
            const auto endpoint = parse("http://user:pass@localhost:8086");
            CHECK(endpoint.host == "localhost");
            CHECK(endpoint.authentication.type == AuthenticationType::Basic);
            CHECK(endpoint.authentication.username == "user");
            CHECK(endpoint.authentication.password == "pass");
            CHECK(endpoint.authentication.apiKey.empty());
        }

        SECTION("IPv4 host")
        {
            const auto endpoint = parse("https://user:pass@192.168.1.10:8086");
            CHECK(endpoint.host == "192.168.1.10");
            CHECK(endpoint.authentication.type == AuthenticationType::Basic);
            CHECK(endpoint.authentication.username == "user");
            CHECK(endpoint.authentication.password == "pass");
        }

        SECTION("IPv6 host")
        {
            const auto endpoint = parse("https://user:pass@[2001:db8::1]:8086");
            CHECK(endpoint.host == "2001:db8::1");
            CHECK(endpoint.authentication.type == AuthenticationType::Basic);
            CHECK(endpoint.authentication.username == "user");
            CHECK(endpoint.authentication.password == "pass");
        }

        SECTION("TCP host")
        {
            const auto endpoint = parse("tcp://user:pass@localhost:8094");
            CHECK(endpoint.host == "localhost");
            CHECK(endpoint.authentication.type == AuthenticationType::Basic);
        }

        SECTION("Password may contain additional colons")
        {
            const auto endpoint = parse("http://user:pa:ss@localhost:8086");
            CHECK(endpoint.authentication.type == AuthenticationType::Basic);
            CHECK(endpoint.authentication.username == "user");
            CHECK(endpoint.authentication.password == "pa:ss");
        }
    }

    TEST_CASE("Parses API key authentication", "[UriParserTest]")
    {
        const auto [uri, expectedHost] = GENERATE(
            std::pair{"http://apikey@localhost:8086", "localhost"},
            std::pair{"https://apikey@192.168.1.10:8086", "192.168.1.10"},
            std::pair{"http://apikey@[::1]:8086", "::1"},
            std::pair{"udp://apikey@localhost:8089", "localhost"},
            std::pair{"tcp://apikey@[::1]:8094", "::1"});

        CAPTURE(uri);

        const auto endpoint = parse(uri);

        CHECK(endpoint.host == expectedHost);
        CHECK(endpoint.authentication.type == AuthenticationType::ApiKey);
        CHECK(endpoint.authentication.apiKey == "apikey");
        CHECK(endpoint.authentication.username.empty());
        CHECK(endpoint.authentication.password.empty());
    }

    TEST_CASE("Authentication is separate from the host", "[UriParserTest]")
    {
        const auto basic = parse("http://user:pass@user.example.com:8086");
        CHECK(basic.host == "user.example.com");
        CHECK(basic.host.find('@') == std::string::npos);
        CHECK(basic.authentication.username == "user");
        CHECK(basic.authentication.password == "pass");

        const auto apiKey = parse("http://apikey@apikey.example.com:8086");
        CHECK(apiKey.host == "apikey.example.com");
        CHECK(apiKey.host.find('@') == std::string::npos);
        CHECK(apiKey.authentication.apiKey == "apikey");

        const auto ipv4 = parse("https://user:pass@192.168.1.10:8086");
        CHECK(ipv4.host == "192.168.1.10");

        const auto ipv6 = parse("https://user:pass@[2001:db8::1]:8086");
        CHECK(ipv6.host == "2001:db8::1");
        CHECK(ipv6.host.find('@') == std::string::npos);
    }

    TEST_CASE("Rejects empty authentication fields", "[UriParserTest]")
    {
        const auto testCase = GENERATE(
            RejectionCase{"http://@localhost:8086", "malformed authentication"},
            RejectionCase{"http://:pass@localhost:8086", "malformed authentication"},
            RejectionCase{"http://user:@localhost:8086", "malformed authentication"},
            RejectionCase{"http://user:pass@", "missing host"});

        checkRejected(testCase);
    }

    TEST_CASE("Handles reserved characters and percent encoding", "[UriParserTest]")
    {
        SECTION("Percent encoding is not decoded")
        {
            const auto basic = parse("http://user:p%40ss@localhost:8086");
            CHECK(basic.authentication.type == AuthenticationType::Basic);
            CHECK(basic.authentication.password == "p%40ss");

            const auto apiKey = parse("http://apikey%2Fpart@localhost:8086");
            CHECK(apiKey.authentication.type == AuthenticationType::ApiKey);
            CHECK(apiKey.authentication.apiKey == "apikey%2Fpart");

            const auto hex = parse("http://user:%41@localhost:8086");
            CHECK(hex.authentication.password == "%41");
        }

        SECTION("Raw reserved characters are rejected")
        {
            const auto testCase = GENERATE(
                RejectionCase{"http://user:pa#ss@localhost:8086", "fragments are not supported"},
                RejectionCase{"http://user:pa/ss@localhost:8086", "malformed authentication"},
                RejectionCase{"http://user:p@ss@localhost:8086", "malformed authentication"},
                RejectionCase{"http://apikey/part@localhost:8086", "malformed authentication"});

            checkRejected(testCase);
        }

        SECTION("Raw question mark is a query boundary, not API key content")
        {
            const auto endpoint = parse("http://apikey?token@localhost:8086");
            CHECK(endpoint.host == "apikey");
            CHECK(endpoint.authentication.type == AuthenticationType::None);
            CHECK(endpoint.authentication.apiKey.empty());

            CHECK_THROWS_WITH(parse("http://user:pa?ss@localhost:8086"),
                              ContainsSubstring("invalid port"));
        }
    }

    TEST_CASE("Validates ports", "[UriParserTest]")
    {
        SECTION("TCP and UDP require a port")
        {
            const auto testCase = GENERATE(
                RejectionCase{"tcp://localhost", "TCP requires a port"},
                RejectionCase{"udp://localhost", "UDP requires a port"},
                RejectionCase{"tcp://[::1]", "TCP requires a port"},
                RejectionCase{"udp://[::1]", "UDP requires a port"});

            checkRejected(testCase);
        }

        SECTION("Invalid ports are rejected")
        {
            const auto testCase = GENERATE(
                RejectionCase{"http://localhost:0", "invalid port"},
                RejectionCase{"http://localhost:65536", "invalid port"},
                RejectionCase{"http://localhost:999999", "invalid port"},
                RejectionCase{"http://localhost:abc", "invalid port"},
                RejectionCase{"http://localhost:", "invalid port"},
                RejectionCase{"tcp://localhost:-1", "invalid port"},
                RejectionCase{"http://localhost:+8086", "invalid port"},
                RejectionCase{"http://localhost:8086x", "invalid port"},
                RejectionCase{"http://localhost: 8086", "invalid port"},
                RejectionCase{"http://localhost:8086.0", "invalid port"},
                RejectionCase{"http://[::1]:", "invalid port"},
                RejectionCase{"http://[::1]:8086:9", "invalid port"});

            checkRejected(testCase);
        }

        SECTION("Port boundaries are accepted")
        {
            CHECK(parse("http://localhost:1").port == 1);
            CHECK(parse("http://localhost:65535").port == 65535);
        }
    }

    TEST_CASE("Rejects malformed IPv6 addresses", "[UriParserTest]")
    {
        const auto testCase = GENERATE(
            RejectionCase{"http://2001:db8::1:8086", "IPv6 address must be bracketed"},
            RejectionCase{"http://[::1", "malformed IPv6 address"},
            RejectionCase{"http://[::1]junk", "malformed IPv6 address"},
            RejectionCase{"http://[]:8086", "malformed IPv6 address"},
            RejectionCase{"http://[localhost]:8086", "malformed IPv6 address"},
            RejectionCase{"http://[::1]:", "invalid port"},
            RejectionCase{"https://user:pass@[::1", "malformed IPv6 address"});

        checkRejected(testCase);
    }

    TEST_CASE("Bracketed IPv6 validation is structural", "[UriParserTest]")
    {
        const auto endpoint = parse("http://[foo:bar]:8086");
        CHECK(endpoint.host == "foo:bar");
        CHECK(endpoint.port == 8086);

        CHECK_THROWS_WITH(parse("http://[foo]:8086"),
                          ContainsSubstring("malformed IPv6 address"));
    }

    TEST_CASE("Rejects malformed endpoints and unsupported schemes", "[UriParserTest]")
    {
        const auto testCase = GENERATE(
            RejectionCase{"", "missing scheme separator"},
            RejectionCase{"localhost:8086", "missing scheme separator"},
            RejectionCase{"http:/localhost:8086", "missing scheme separator"},
            RejectionCase{"://localhost:8086", "missing scheme separator"},
            RejectionCase{"ftp://localhost:8086", "unsupported scheme"},
            RejectionCase{"HTTP://localhost:8086", "unsupported scheme"},
            RejectionCase{"ws://localhost:8086", "unsupported scheme"},
            RejectionCase{"http://", "missing host"},
            RejectionCase{"http://:8086", "missing host"},
            RejectionCase{"http:///path", "missing host"},
            RejectionCase{"http://user:pass@", "missing host"},
            RejectionCase{"http://user:pass@:8086", "missing host"},
            RejectionCase{"tcp://localhost:8094/path", "path is not supported for this scheme"},
            RejectionCase{"udp://localhost:8089/path", "path is not supported for this scheme"},
            RejectionCase{"tcp://localhost:8094?x=1", "query strings are not supported for this scheme"},
            RejectionCase{"udp://localhost:8089?x=1", "query strings are not supported for this scheme"},
            RejectionCase{"http://localhost:8086#frag", "fragments are not supported"},
            RejectionCase{"unix:///tmp/x.sock#frag", "fragments are not supported"});

        checkRejected(testCase);
    }

    TEST_CASE("Rejects invalid Unix socket paths", "[UriParserTest]")
    {
        const auto testCase = GENERATE(
            RejectionCase{"unix://", "Unix socket path must be absolute"},
            RejectionCase{"unix://tmp/x.sock", "Unix socket path must be absolute"},
            RejectionCase{"unix://relative", "Unix socket path must be absolute"},
            RejectionCase{"unix:/tmp/x.sock", "missing scheme separator"},
            RejectionCase{"unix:///tmp/x.sock?db=test", "query strings are not supported for this scheme"});

        checkRejected(testCase);
    }

    TEST_CASE("Errors do not contain credentials", "[UriParserTest]")
    {
        struct LeakCase
        {
            std::string_view input;
            std::string_view username;
            std::string_view secret;
        };

        const auto testCase = GENERATE(
            LeakCase{"http://user:supersecret@", "user", "supersecret"},
            LeakCase{"http://apikey-supersecret@localhost:0", "", "apikey-supersecret"},
            LeakCase{"http://user:supersecret@[::1", "user", "supersecret"},
            LeakCase{"http://user:supersecret@localhost:8086#frag", "user", "supersecret"});

        CAPTURE(testCase.input);

        const auto message = getExceptionMessage(testCase.input);

        CHECK(message.find(testCase.secret) == std::string::npos);

        if (!testCase.username.empty())
        {
            CHECK(message.find(testCase.username) == std::string::npos);
        }

        CHECK(message.find(testCase.input) == std::string::npos);
    }
}
