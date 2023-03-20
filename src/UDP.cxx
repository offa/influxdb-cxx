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

#include "UDP.h"
#include "InfluxDBException.h"
#include <string>

namespace influxdb::transports
{

    UDP::UDP(const std::string& hostname, std::uint16_t port)
        : mSocket(mIoService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0))
    {
        boost::asio::ip::udp::resolver resolver(mIoService);
        try
        {
            // "A successful call to this function is guaranteed to return a non-empty range."
            // https://www.boost.org/doc/libs/1_81_0/doc/html/boost_asio/reference/ip__basic_resolver/resolve/overload7.html
            mEndpoint = *(resolver.resolve(boost::asio::ip::udp::v4(), hostname, std::to_string(port)));
        }
        catch (const boost::system::system_error& e)
        {
            throw InfluxDBException(e.what());
        }
    }

    void UDP::send(std::string&& message)
    {
        try
        {
            mSocket.send_to(boost::asio::buffer(message, message.size()), mEndpoint);
        }
        catch (const boost::system::system_error& e)
        {
            throw InfluxDBException(e.what());
        }
    }

    std::size_t UDP::getMaxMessageSize() const
    {
        // UDP header has a 16-bit length field
        static constexpr std::size_t maxLengthValue{std::numeric_limits<std::uint16_t>::max()};
        static constexpr std::size_t udpHeaderSize{8};
        // Currently only IPv4 is supported
        static constexpr std::size_t ipv4HeaderSize{20};
        // Max UDP data size for IPv4 is 65535 - 8 - 20 = 65507
        static constexpr std::size_t maxUDPDataSize{maxLengthValue - udpHeaderSize - ipv4HeaderSize};
        return maxUDPDataSize;
    }

} // namespace influxdb::transports
