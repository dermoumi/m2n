/*
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
*/

#pragma once
#include "../config.hpp"

#include <string>

// A set of functions to handle logging information
class NX_HIDDEN Log
{
public:
    static bool setLogFile(const std::string& filename);

    static void verbose(const char* format, ...);
    static void verbose(const std::string& message)
    {
        verbose(message.data());
    }

    static void debug(const char* format, ...);
    static void debug(const std::string& message)
    {
        debug(message.data());
    }

    static void info(const char* format, ...);
    static void info(const std::string& message)
    {
        info(message.data());
    }

    static void warning(const char* format, ...);
    static void warning(const std::string& message)
    {
        warning(message.data());
    }

    static void error(const char* format, ...);
    static void error(const std::string& message)
    {
        error(message.data());
    }

    static void fatal(const char* format, ...);
    static void fatal(const std::string& message)
    {
        fatal(message.data());
    }
};
