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

#include "TCP.h"
#include "InfluxDB/InfluxDBException.h"
#include <string>

namespace influxdb::transports
{
    namespace ba = boost::asio;

    TCP::TCP(const std::string& hostname, int port)
        : mSocket(mIoService)
    {
        ba::ip::tcp::resolver resolver(mIoService);
        ba::ip::tcp::resolver::query query(hostname, std::to_string(port));
        ba::ip::tcp::resolver::iterator resolverIterator = resolver.resolve(query);
        ba::ip::tcp::resolver::iterator end;
        mEndpoint = *resolverIterator;
        mSocket.open(mEndpoint.protocol());
        reconnect();
    }

    bool TCP::is_connected() const
    {
        return mSocket.is_open();
    }

    void TCP::reconnect()
    {
        mSocket.connect(mEndpoint);
        mSocket.wait(ba::ip::tcp::socket::wait_write);
    }

    void TCP::send(std::string&& message)
    {
        try
        {
            message.append("\n");
            const size_t written = mSocket.write_some(ba::buffer(message, message.size()));
            if (written != message.size())
            {
                throw InfluxDBException("Error while transmitting data");
            }
        }
        catch (const boost::system::system_error& e)
        {
            throw InfluxDBException(e.what());
        }
    }

} // namespace influxdb::transports
