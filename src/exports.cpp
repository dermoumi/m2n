#include "config.hpp"
#include "log.hpp"
#include "filesystem.hpp"
#include <physfs/physfs.h>

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

    //--------------------------------------------------------------------------

    struct PHYSFS_File;

    NX_EXPORT const char* nxFsGetError()
    {
        thread_local std::string message;

        auto errorMessage = PHYSFS_getLastError();
        message = errorMessage ? errorMessage : "";

        return message.data();
    }

    NX_EXPORT bool nxFsMountDir(const char* dir, const char* mountPoint, bool append)
    {
        return Filesystem::mountDir(dir, mountPoint, append);
    }

    NX_EXPORT bool nxFsMountArchive(const char* file, const char* mountPoint, bool append)
    {
        return Filesystem::mountArchive(file, mountPoint, append);
    }

    NX_EXPORT PHYSFS_File* nxFsOpenRead(const char* fileName)
    {
        return PHYSFS_openRead(fileName);
    }

    NX_EXPORT PHYSFS_File* nxFsOpenWrite(const char* fileName)
    {
        return PHYSFS_openWrite(fileName);
    }

    NX_EXPORT void nxFsClose(PHYSFS_File* handle)
    {
        if (handle) {
            PHYSFS_close(handle);
        }
    }

    NX_EXPORT bool nxFsRead(PHYSFS_File* handle, char* buffer, size_t size, size_t* readBytes)
    {
        auto status = PHYSFS_readBytes(handle, buffer, size);
        if (status < 0) return false;

        if (readBytes) *readBytes = static_cast<size_t>(status);

        return true;
    }

    NX_EXPORT size_t nxFsSize(PHYSFS_File* handle)
    {
        auto size = PHYSFS_fileLength(handle);
        return static_cast<size_t>(size);
    }
}