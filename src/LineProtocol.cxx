// MIT License
//
// Copyright (c) 2020-2023 offa
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

#include "LineProtocol.h"

#include <iomanip>
#include <sstream>

namespace influxdb
{
    namespace
    {
        template <class... Ts>
        struct overloaded : Ts...
        {
            using Ts::operator()...;
        };
        template <class... Ts>
        overloaded(Ts...) -> overloaded<Ts...>;

        void appendIfNotEmpty(std::string& dest, const std::string& value, char separator)
        {
            if (!value.empty())
            {
                dest.append(std::string{separator}).append(value);
            }
        }

        std::string escapeCharacters(std::string_view input, const std::string& escapedChars)
        {
            static const std::string escapeCharacter{"\\"};
            std::string output;
            output.reserve(input.size());

            std::size_t searchStartPos{0};
            // Find the first character that needs to be escaped
            std::size_t escapedCharacterPos{input.find_first_of(escapedChars, searchStartPos)};
            while (escapedCharacterPos != std::string::npos)
            {
                // Append the characters between the previous escaped character and the current one
                output.append(input, searchStartPos, escapedCharacterPos - searchStartPos);
                // Append the escape character and the character to be escaped
                output.append(escapeCharacter).append(1, input[escapedCharacterPos]);
                // Update the search start index to the character after the escaped character
                searchStartPos = escapedCharacterPos + 1;
                // Find the next character that needs to be escaped
                escapedCharacterPos = input.find_first_of(escapedChars, searchStartPos);
            }
            // Append remaining characters after the final escaped character
            output.append(input, searchStartPos);

            return output;
        }

        std::string formatTags(const Point::TagSet& tagsDeque)
        {
            std::string tags;
            bool addComma{false};
            for (const auto& tag : tagsDeque)
            {
                if (addComma)
                {
                    tags += ',';
                }
                tags += LineProtocol::EscapeStringElement(LineProtocol::ElementType::TagKey, tag.first);
                tags += '=';
                tags += LineProtocol::EscapeStringElement(LineProtocol::ElementType::TagValue, tag.second);
                addComma = true;
            }

            return tags;
        }

        std::string formatFields(const Point::FieldSet& fieldsDeque)
        {
            std::stringstream convert;
            convert << std::setprecision(Point::floatsPrecision) << std::fixed;
            bool addComma{false};
            for (const auto& field : fieldsDeque)
            {
                if (addComma)
                {
                    convert << ',';
                }

                convert << LineProtocol::EscapeStringElement(LineProtocol::ElementType::FieldKey, field.first) << "=";
                std::visit(overloaded{
                               [&convert](int v)
                               { convert << v << 'i'; },
                               [&convert](long long int v)
                               { convert << v << 'i'; },
                               [&convert](double v)
                               { convert << v; },
                               [&convert](const std::string& v)
                               { convert << '"' << LineProtocol::EscapeStringElement(LineProtocol::ElementType::FieldValue, v) << '"'; },
                               [&convert](bool v)
                               { convert << (v ? "true" : "false"); },
                               [&convert](unsigned int v)
                               { convert << v << 'u'; },
                               [&convert](unsigned long long int v)
                               { convert << v << 'u'; },
                           },
                           field.second);
                addComma = true;
            }

            return convert.str();
        }
    }
    LineProtocol::LineProtocol()
        : LineProtocol(std::string{})
    {
    }

    LineProtocol::LineProtocol(const std::string& tags)
        : globalTags(tags)
    {
    }

    std::string LineProtocol::format(const Point& point) const
    {
        std::string line{LineProtocol::EscapeStringElement(LineProtocol::ElementType::Measurement, point.getName())};
        appendIfNotEmpty(line, globalTags, ',');
        appendIfNotEmpty(line, formatTags(point.getTagSet()), ',');
        appendIfNotEmpty(line, formatFields(point.getFieldSet()), ' ');

        return line.append(" ")
            .append(std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(point.getTimestamp().time_since_epoch()).count()));
    }

    std::string LineProtocol::EscapeStringElement(LineProtocol::ElementType type, std::string_view element)
    {
        // https://docs.influxdata.com/influxdb/cloud/reference/syntax/line-protocol/#special-characters
        static const std::string commaAndSpace{", "};
        static const std::string commaEqualsAndSpace{",= "};
        static const std::string doubleQuoteAndBackslash{R"("\)"};

        switch (type)
        {
            case ElementType::Measurement:
                return escapeCharacters(element, commaAndSpace);
            case ElementType::TagKey:
            case ElementType::TagValue:
            case ElementType::FieldKey:
                return escapeCharacters(element, commaEqualsAndSpace);
            case ElementType::FieldValue:
                return escapeCharacters(element, doubleQuoteAndBackslash);
        }
        return std::string{element};
    }
}
