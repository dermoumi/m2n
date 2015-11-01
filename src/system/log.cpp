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
#include "log.hpp"

#include <SDL2/SDL.h>

#include <cstdarg>
#include <cstdio>
#include <memory>
#include <mutex>

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
namespace
{
    struct FileDeleter
    {
        void operator()(void* file)
        {
            if (file != nullptr) fclose(static_cast<FILE*>(file));
        }
    };
    using FilePtr = std::unique_ptr<FILE, FileDeleter>;

    std::mutex logFileMutex;
    FilePtr logFile {nullptr};

    void logToFile(const char* prefix, const char* format, va_list args)
    {
        std::lock_guard<std::mutex> lock(logFileMutex);
        if (!logFile) return;

        auto* file = logFile.get();

        fprintf (file, "%s: ", prefix); // Write prefix
        vfprintf(file, format, args);   // Write message
        fprintf (file, "\r\n");         // New line
        fflush  (file);                 // Flush
    }
}

//----------------------------------------------------------
bool Log::setLogFile(const std::string& filename)
{
    logFile = FilePtr(fopen(filename.data(), "w"));
    return logFile != nullptr;
}

//----------------------------------------------------------
void Log::verbose(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, format, args);
    logToFile("VERBOSE", format, args);

    va_end(args);
}

//----------------------------------------------------------
void Log::debug(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, format, args);
    logToFile("DEBUG", format, args);
    
    va_end(args);
}

//----------------------------------------------------------
void Log::info(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, format, args);
    logToFile("INFO", format, args);
    
    va_end(args);
}

//----------------------------------------------------------
void Log::warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, format, args);
    logToFile("WARN", format, args);
    
    va_end(args);
}

//----------------------------------------------------------
void Log::error(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, format, args);
    logToFile("ERROR", format, args);
    
    va_end(args);
}

//----------------------------------------------------------
void Log::fatal(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, format, args);
    logToFile("FATAL", format, args);
    
    va_end(args);
}

//==============================================================================
