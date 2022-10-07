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

#pragma once

#include <curl/curl.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_version_macros.hpp>
#include <catch2/trompeloeil.hpp>


namespace influxdb::test
{
    struct CurlHandleDummy
    {
    };

    using WriteCallbackFn = size_t (*)(void*, size_t, size_t, void*);


    struct CurlMock
    {
        MAKE_MOCK1(curl_global_init, CURLcode(long));
        MAKE_MOCK0(curl_easy_init, CURL*());
        MAKE_MOCK3(curl_easy_setopt_, CURLcode(CURL*, CURLoption, long));
        MAKE_MOCK3(curl_easy_setopt_, CURLcode(CURL*, CURLoption, unsigned long));
        MAKE_MOCK3(curl_easy_setopt_, CURLcode(CURL*, CURLoption, std::string));
        MAKE_MOCK3(curl_easy_setopt_, CURLcode(CURL*, CURLoption, void*));
        MAKE_MOCK3(curl_easy_setopt_, CURLcode(CURL*, CURLoption, WriteCallbackFn));
        MAKE_MOCK1(curl_easy_cleanup, void(CURL*));
        MAKE_MOCK0(curl_global_cleanup, void());
        MAKE_MOCK1(curl_easy_perform, CURLcode(CURL* easy_handle));
        MAKE_MOCK3(curl_easy_getinfo_, CURLcode(CURL*, CURLINFO, long*));
        MAKE_MOCK3(curl_easy_escape, char*(CURL*, const char*, int) );
        MAKE_MOCK1(curl_free, void(void*));
    };

    extern CurlMock curlMock;
}
