// MIT License
//
// Copyright (c) 2020-2023 offa
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

#ifndef INFLUXDATA_TRANSPORTS_HTTP_H
#define INFLUXDATA_TRANSPORTS_HTTP_H

#include "Transport.h"
#include <memory>
#include <string>
#include <cpr/cpr.h>

namespace influxdb::transports
{

    /// \brief HTTP transport
    class HTTP : public Transport
    {
    public:
        /// Constructor
        explicit HTTP(const std::string& url);

        /// Sends point via HTTP POST
        ///  \throw InfluxDBException	when send fails
        void send(std::string&& lineprotocol) override;

        /// Queries database
        /// \throw InfluxDBException	when query fails
        std::string query(const std::string& query) override;

        /// Execute command
        /// \throw InfluxDBException    when execution fails
        std::string execute(const std::string& cmd) override;

        /// Creates database used at url if it does not exists
        /// \throw InfluxDBException	when HTTP POST fails
        void createDatabase() override;

        /// Enable Basic Auth
        /// \param auth <username>:<password>
        void enableBasicAuth(const std::string& auth);

        /// Sets proxy
        void setProxy(const Proxy& proxy) override;

    private:
        /// InfluxDB service URL
        std::string mInfluxDbServiceUrl;

        /// Database name used
        std::string mDatabaseName;

        cpr::Session session;
    };

} // namespace influxdb

#endif // INFLUXDATA_TRANSPORTS_HTTP_H
