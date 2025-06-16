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

#include "InfluxDB/Transport.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/trompeloeil.hpp>

namespace influxdb::test
{
    class TransportMock : public Transport
    {
        MAKE_MOCK1(send, void(std::string&&), override);
        MAKE_MOCK1(query, std::string(const std::string&), override);
        MAKE_MOCK0(createDatabase, void(), override);
        MAKE_MOCK1(execute, std::string(const std::string&), override);
        MAKE_MOCK1(setTimePrecision, void(TimePrecision), override);
    };


    class TransportAdapter : public Transport
    {
    public:
        explicit TransportAdapter(std::shared_ptr<TransportMock> mock)
            : mockImpl(mock)
        {
        }

        void send(std::string&& message) override
        {
            mockImpl->send(std::move(message));
        }

        std::string query(const std::string& query) override
        {
            return mockImpl->query(query);
        }

        std::string execute(const std::string& cmd) override
        {
            return mockImpl->execute(cmd);
        }

        void createDatabase() override
        {
            mockImpl->createDatabase();
        }

        void setTimePrecision(TimePrecision precision) override
        {
            mockImpl->setTimePrecision(precision);
        }

    private:
        std::shared_ptr<TransportMock> mockImpl;
    };

}
