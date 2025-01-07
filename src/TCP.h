// MIT License
//
// Copyright (c) 2021 Felix Moessbauer
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
/// \author Felix Moessbauer
///

#ifndef INFLUXDATA_TRANSPORTS_TCP_H
#define INFLUXDATA_TRANSPORTS_TCP_H

#include "Transport.h"

#include <boost/asio.hpp>
#include <chrono>
#include <string>

namespace influxdb::transports
{

    /// \brief TCP transport
    class TCP : public Transport
    {
    public:
        /// Constructor
        TCP(const std::string& hostname, int port);

        /// Sends blob via TCP
        void send(std::string&& message) override;

        /// check if socket is connected
        bool is_connected() const;

        /// reconnect socket
        void reconnect();

    private:
        /// Boost Asio I/O functionality
        boost::asio::io_context mIoContext;

        /// TCP socket
        boost::asio::ip::tcp::socket mSocket;

        /// TCP endpoint
        boost::asio::ip::tcp::endpoint mEndpoint;
    };

} // namespace influxdb::transports

#endif // INFLUXDATA_TRANSPORTS_TCP_H
