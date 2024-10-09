// MIT License
//
// Copyright (c) 2020-2024 offa
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

#ifndef INFLUXDATA_INFLUXDB_FACTORY_H
#define INFLUXDATA_INFLUXDB_FACTORY_H

#include "InfluxDB.h"
#include "InfluxDBTransport.h"
#include "influxdb_export.h"

namespace influxdb
{

    /// \brief InfluxDB factory
    class INFLUXDB_EXPORT InfluxDBFactory
    {
    public:
        /// Disables copy constructor
        InfluxDBFactory& operator=(const InfluxDBFactory&) = delete;

        /// Disables copy constructor
        InfluxDBFactory(const InfluxDBFactory&) = delete;

        /// InfluxDB factory
        /// Provides InfluxDB instance with given transport
        /// \param url   URL defining transport details
        /// \throw InfluxDBException     if unrecognised backend or missing protocol
        static std::unique_ptr<InfluxDB> Get(const std::string& url) noexcept(false);

        /// InfluxDB factory
        /// Provides InfluxDB instance with given transport and proxy
        /// \param url   URL defining transport details
        /// \param proxy   Proxy
        /// \throw InfluxDBException     if unrecognised backend, missing protocol or unsupported proxy
        static std::unique_ptr<InfluxDB> Get(const std::string& url, const Proxy& proxy);

    private:
        ///\return  backend based on provided URL
        static std::unique_ptr<Transport> GetTransport(const std::string& url);

        /// Private constructor disallows to create instance of Factory
        InfluxDBFactory() = default;
    };

} // namespace influxdb

#endif // INFLUXDATA_INFLUXDB_FACTORY_H
