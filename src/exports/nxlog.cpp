#include "../config.hpp"
#include "../log.hpp"

extern "C"
{
    NX_EXPORT void nxLogVerbose(const char* message)
    {
        Log::verbose(message);
    }

    NX_EXPORT void nxLogDebug(const char* message)
    {
        Log::debug(message);
    }

    NX_EXPORT void nxLogInfo(const char* message)
    {
        Log::info(message);
    }

    NX_EXPORT void nxLogError(const char* message)
    {
        Log::error(message);
    }

    NX_EXPORT void nxLogFatal(const char* message)
    {
        Log::fatal(message);
    }
}