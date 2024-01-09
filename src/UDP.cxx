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
#include <algorithm>
#include <iterator>

namespace influxdb::transports
{
    namespace
    {
        std::size_t getSocketSendBufferSize(const boost::asio::ip::udp::socket& socket)
        {
            boost::asio::ip::udp::socket::send_buffer_size sendBufferSizeOption;
            socket.get_option(sendBufferSizeOption);
            int sendBufferSize{sendBufferSizeOption.value()};
            return (sendBufferSize >= 0 ? static_cast<std::size_t>(sendBufferSize) : 0U);
        }

        std::size_t getMaxMessageSize(const boost::asio::ip::udp::socket& s)
        {
            constexpr std::size_t maxLengthValue{(std::numeric_limits<std::uint16_t>::max)()};
            constexpr std::size_t udpHeaderSize{8};
            constexpr std::size_t ipv4HeaderSize{20};
            constexpr std::size_t maxUDPDataSize{maxLengthValue - udpHeaderSize - ipv4HeaderSize};

            return std::min(maxUDPDataSize, getSocketSendBufferSize(s));
        }


        template <class T>
        auto first(T itr1, T itr2)
        {
            return std::distance(itr1, itr2) > 0 ? itr1 : itr2;
        }

        void sendChunks(boost::asio::ip::udp::socket& socket, boost::asio::ip::udp::endpoint& endpoint, std::string message, std::size_t limit)
        {
            auto begin = message.begin();
            while (true)
            {
                const auto end = first(std::next(begin, limit), message.end());

                if (end == message.end())
                {
                    socket.send_to(boost::asio::buffer(std::string{begin, end}), endpoint);
                    return;
                }

                const auto eol = std::find(std::make_reverse_iterator(end), std::make_reverse_iterator(begin), '\n');

                if (eol.base() != begin)
                {
                    socket.send_to(boost::asio::buffer(std::string{begin, eol.base()}), endpoint);
                    begin = std::next(eol.base());
                }
                else
                {
                    begin = std::next(std::find(end, message.end(), '\n'));
                }
            }
        }

    }


    UDP::UDP(const std::string& hostname, int port)
        : mSocket(mIoService, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)),
          maxMessageSize(getMaxMessageSize(mSocket))
    {
        boost::asio::ip::udp::resolver resolver(mIoService);
        boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), hostname, std::to_string(port));
        boost::asio::ip::udp::resolver::iterator resolverInerator = resolver.resolve(query);
        mEndpoint = *resolverInerator;
    }

    void UDP::send(std::string&& message)
    {
        try
        {
            if (message.size() < maxMessageSize)
            {
                mSocket.send_to(boost::asio::buffer(message, message.size()), mEndpoint);
            }
            else
            {
                sendChunks(mSocket, mEndpoint, std::move(message), maxMessageSize);
            }
        }
        catch (const boost::system::system_error& e)
        {
            throw InfluxDBException(e.what());
        }
    }

} // namespace influxdb::transports
