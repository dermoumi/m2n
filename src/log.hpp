#pragma once
#include "config.hpp"
#include <string>

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
