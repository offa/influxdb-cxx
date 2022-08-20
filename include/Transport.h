// MIT License
//
// Copyright (c) 2020-2022 offa
// Copyright (c) 2019 Adam Wegrzynek
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

///
/// \author Adam Wegrzynek
///

#ifndef INFLUXDATA_TRANSPORTINTERFACE_H
#define INFLUXDATA_TRANSPORTINTERFACE_H

#include "InfluxDBException.h"
#include "influxdb_export.h"
#include "Proxy.h"

namespace influxdb
{

    /// \brief Transport interface
    class INFLUXDB_EXPORT Transport
    {
    public:
        Transport() = default;

        virtual ~Transport() = default;

        /// Sends string blob
        virtual void send(std::string&& message) = 0;

        /// Sends request
        virtual std::string query([[maybe_unused]] const std::string& query)
        {
            throw InfluxDBException{"Transport", "Queries are not supported by the selected transport"};
        }

        /// Sends request
        virtual void createDatabase()
        {
            throw InfluxDBException{"Transport", "Creation of database is not supported by the selected transport"};
        }

        /// Sets proxy
        virtual void setProxy([[maybe_unused]] Proxy proxy)
        {
            throw InfluxDBException{"Transport", "Proxy is not supported by the selected transport"};
        }
    };

} // namespace influxdb

#endif // INFLUXDATA_TRANSPORTINTERFACE_H
