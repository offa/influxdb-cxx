// MIT License
//
// Copyright (c) 2020-2022 offa
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

#ifndef INFLUXDATA_PROXY_H
#define INFLUXDATA_PROXY_H

#include "influxdb_export.h"
#include <string>
#include <optional>

namespace influxdb
{
    class INFLUXDB_EXPORT Proxy
    {
    public:
        struct Auth
        {
            std::string user;
            std::string password;
        };


        Proxy(const std::string& proxy, Auth auth);
        explicit Proxy(const std::string& proxy);

        const std::string& getProxy() const;
        std::optional<Auth> getAuthentication() const;

    private:
        std::string proxy_;
        std::optional<Auth> auth_;
    };
}
#endif /* INFLUXDATA_PROXY_H */
