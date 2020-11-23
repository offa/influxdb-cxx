// MIT License
//
// Copyright (c) 2020 offa
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

#include "CurlMock.h"
#include <stdarg.h>

void curl_easy_cleanup(CURL* handle)
{
    influxdb::test::curlMock.curl_easy_cleanup(handle);
}

char* curl_easy_escape(CURL* curl, const char* string, int length)
{
    return influxdb::test::curlMock.curl_easy_escape(curl, string, length);
}

const char* curl_easy_strerror([[maybe_unused]] CURLcode errornum)
{
    return "Mocked error message";
}

void curl_global_cleanup()
{
    influxdb::test::curlMock.curl_global_cleanup();
}

CURL* curl_easy_init()
{
    return influxdb::test::curlMock.curl_easy_init();
}

CURLcode curl_easy_setopt(CURL* handle, CURLoption option, ...)
{
    switch (option)
    {
        case CURLOPT_WRITEDATA:
        {
            va_list argp;
            va_start(argp, option);
            void* outValue = va_arg(argp, void*);
            va_end(argp);
            return influxdb::test::curlMock.curl_easy_setopt_(handle, option, outValue);
        }
        case CURLOPT_URL:
        {
            va_list argp;
            va_start(argp, option);
            const std::string value = va_arg(argp, const char*);
            va_end(argp);
            return influxdb::test::curlMock.curl_easy_setopt_(handle, option, value);
        }
        default:
            return influxdb::test::curlMock.curl_easy_setopt_(handle, option);
    }
}

CURLcode curl_easy_perform(CURL* easy_handle)
{
    return influxdb::test::curlMock.curl_easy_perform(easy_handle);
}

void curl_free(void* ptr)
{
    static_cast<void>(ptr);
}

CURLcode curl_global_init(long flags)
{
    return influxdb::test::curlMock.curl_global_init(flags);
}

CURLcode curl_easy_getinfo(CURL* curl, CURLINFO info, ...)
{
    va_list argp;
    va_start(argp, info);
    long* outValue = va_arg(argp, long*);
    const auto result = influxdb::test::curlMock.curl_easy_getinfo_(curl, info, outValue);
    va_end(argp);
    return result;
}
