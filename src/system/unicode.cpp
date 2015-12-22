/*//============================================================================
    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org>
*///============================================================================
#include "unicode.hpp"

#include <iterator>

template <typename In>
static In decodeUtf8(In begin, In end, uint32_t& output, uint32_t rep)
{
    static const int trailing[256] {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
    };

    static const uint32_t offsets[6] {
        0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080
    };

    // decode the character
    int trailingBytes = trailing[static_cast<uint8_t>(*begin)];

    if (begin + trailingBytes < end) {
        output = 0;
        switch(trailingBytes) {
            case 5: output += static_cast<uint8_t>(*begin++); output <<= 6;
            case 4: output += static_cast<uint8_t>(*begin++); output <<= 6;
            case 3: output += static_cast<uint8_t>(*begin++); output <<= 6;
            case 2: output += static_cast<uint8_t>(*begin++); output <<= 6;
            case 1: output += static_cast<uint8_t>(*begin++); output <<= 6;
            case 0: output += static_cast<uint8_t>(*begin++);
        }
        output -= offsets[trailingBytes];
    }
    else {
        // Incomplete character
        begin = end;
        output = rep;
    }

    return begin;
}

template <typename Out>
static Out encodeUtf8(uint32_t input, Out output)
{
    // Some useful precomputed data
    static const uint8_t firstBytes[7] { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

    if (input <= 0x0010FFFF && (input < 0xD800 || input > 0xDBFF)) {
        // Valid character

        // Get the number of bytes to write
        std::size_t bytesToWrite = 1;
        if      (input <  0x80)       bytesToWrite = 1;
        else if (input <  0x800)      bytesToWrite = 2;
        else if (input <  0x10000)    bytesToWrite = 3;
        else if (input <= 0x0010FFFF) bytesToWrite = 4;

        // Extract the bytes to write
        uint8_t bytes[4];
        switch (bytesToWrite)
        {
            case 4: bytes[3] = static_cast<uint8_t>((input | 0x80) & 0xBF); input >>= 6;
            case 3: bytes[2] = static_cast<uint8_t>((input | 0x80) & 0xBF); input >>= 6;
            case 2: bytes[1] = static_cast<uint8_t>((input | 0x80) & 0xBF); input >>= 6;
            case 1: bytes[0] = static_cast<uint8_t> (input | firstBytes[bytesToWrite]);
        }

        // Add them to the output
        output = std::copy(bytes, bytes + bytesToWrite, output);
    }

    return output;
}

namespace Unicode
{
    std::u32string utf8To32(const std::string& str)
    {
        std::u32string output;
        output.reserve(str.length());

        auto begin = str.begin();
        auto end = str.end();

        while (begin < end) {
            uint32_t codepoint;
            begin = decodeUtf8(begin, end, codepoint, U'?');
            output += codepoint;
        }

        return output;
    }

    std::string utf32To8(const std::u32string& str)
    {
        std::string output;
        output.reserve(str.length());

        auto begin = str.begin();
        auto end   = str.end();
        auto out   = std::back_inserter(output);

        while (begin < end) {
            out = encodeUtf8(*begin++, out);
        }

        return output;
    }
}