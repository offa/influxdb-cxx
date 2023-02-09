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
/// \author Adam Wegrzynek <adam.wegrzynek@cern.ch>
///

#ifndef INFLUXDATA_EXCEPTION_H
#define INFLUXDATA_EXCEPTION_H

#include <stdexcept>
#include <string>

#include "influxdb_export.h"

namespace influxdb
{

    class INFLUXDB_EXPORT InfluxDBException : public std::runtime_error
    {
    public:
        /// \deprecated Use InfluxDBException(const std::string&) instead - will be removed in v0.8.0
        [[deprecated("Use InfluxDBException(const std::string&) instead - will be removed in v0.8.0")]] InfluxDBException(const std::string& source, const std::string& message)
            : std::runtime_error::runtime_error(
                  "influx-cxx [" + source + "]: " + message)
        {
        }

        explicit InfluxDBException(const std::string& message)
            : std::runtime_error::runtime_error(message)
        {
        }
    };


    /// \deprecated Use InfluxDBException instead - will be removed in v0.8.0
    class [[deprecated("Use InfluxDBException instead - will be removed in v0.8.0")]] NonExistentDatabase : public InfluxDBException{
        public :
            NonExistentDatabase(const std::string& source, const std::string& message) : InfluxDBException(source, message){}
    };

    /// \deprecated Use InfluxDBException instead - will be removed in v0.8.0
    class [[deprecated("Use InfluxDBException instead - will be removed in v0.8.0")]] BadRequest : public InfluxDBException{
        public :
            BadRequest(const std::string& source, const std::string& message) : InfluxDBException(source, message){}
    };

    /// \deprecated Use InfluxDBException instead - will be removed in v0.8.0
    class [[deprecated("Use InfluxDBException instead - will be removed in v0.8.0")]] ServerError : public InfluxDBException{
        public :
            ServerError(const std::string& source, const std::string& message) : InfluxDBException(source, message){}
    };

    /// \deprecated Use InfluxDBException instead - will be removed in v0.8.0
    class [[deprecated("Use InfluxDBException instead - will be removed in v0.8.0")]] ConnectionError : public InfluxDBException
    {
    public:
        ConnectionError(const std::string& source, const std::string& message)
            : InfluxDBException(source, message){};
    };


} // namespace influxdb

#endif // INFLUXDATA_EXCEPTION_H
