#include "log.hpp"
#include <SDL2/SDL.h>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <mutex>

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
        if (!logFile)
            return;

        fprintf (logFile.get(), "%s: ", prefix); // Write prefix
        vfprintf(logFile.get(), format, args);   // Write message
        fprintf (logFile.get(), "\r\n");         // New line
        fflush  (logFile.get());                 // Flush
    }
}

bool Log::setLogFile(const std::string& filename)
{
    logFile = FilePtr(fopen(filename.data(), "w"));
    return logFile != nullptr;
}

void Log::verbose(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, format, args);
    logToFile("VERBOSE", format, args);

    va_end(args);
}

void Log::debug(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, format, args);
    logToFile("DEBUG", format, args);
    
    va_end(args);
}

void Log::info(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, format, args);
    logToFile("INFO", format, args);
    
    va_end(args);
}

void Log::warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_WARN, format, args);
    logToFile("WARN", format, args);
    
    va_end(args);
}

void Log::error(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, format, args);
    logToFile("ERROR", format, args);
    
    va_end(args);
}

void Log::fatal(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL, format, args);
    logToFile("FATAL", format, args);
    
    va_end(args);
}
