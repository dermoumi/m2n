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

#include "filesystem.hpp"
#include "log.hpp"

#include <physfs/physfs.h>
#include <SDL2/SDL.h>
#include <sstream>
#include <mutex>

// Implementation of Android's asset management into PHYSFS
#if defined(NX_SYSTEM_ANDROID)
#include <jni.h>
namespace
{
    static PHYSFS_Io* assetsIO = nullptr;

    static jclass mainClassObj = nullptr;
    static jmethodID fsListAssetDirectoryMID = nullptr;
    static jmethodID fsIsDirectoryMID = nullptr;

    int ioSeekFunc(PHYSFS_Io*, PHYSFS_uint64)
    {
        // Nothing to do
        return 1;
    }

    PHYSFS_sint64 ioTellFunc(PHYSFS_Io*)
    {
        // Nothing to do
        PHYSFS_setErrorCode(PHYSFS_ERR_IO);
        return -1;
    }

    PHYSFS_sint64 ioSizeFunc(PHYSFS_Io*)
    {
        // Nothing to do
        PHYSFS_setErrorCode(PHYSFS_ERR_IO);
        return -1;
    }

    PHYSFS_Io* ioDuplicateFunc(PHYSFS_Io*)
    {
        // Nothing to do
        PHYSFS_setErrorCode(PHYSFS_ERR_IO);
        return nullptr;
    }

    void ioDestroyFunc(PHYSFS_Io*)
    {
        // Nothing to do
    }

    struct FileHandle
    {
        SDL_RWops* source;
        PHYSFS_sint64 size;    // Cached size
        std::string filename;  // Needed for handle duplication
    };

    PHYSFS_sint64 readFunc(PHYSFS_Io* io, void* buffer, PHYSFS_uint64 len)
    {
        auto* handle = reinterpret_cast<FileHandle*>(io->opaque);
        auto* source = handle->source;

        // Make sure we won't read past End Of File
        const PHYSFS_sint64 bytesLeft = handle->size - SDL_RWtell(source);
        if (bytesLeft < len) len = bytesLeft;

        size_t status = SDL_RWread(source, buffer, 1u, len);
        if (!status && bytesLeft != len) {
            PHYSFS_setErrorCode(PHYSFS_ERR_IO);
            return -1;
        }

        return len;
    }

    int seekFunc(PHYSFS_Io* io, PHYSFS_uint64 offset)
    {
        auto* handle = reinterpret_cast<FileHandle*>(io->opaque);
        if (handle->size < offset) {
            PHYSFS_setErrorCode(PHYSFS_ERR_PAST_EOF);
            return 0;
        }

        auto* source = handle->source;
        Sint64 status = SDL_RWseek(source, offset, RW_SEEK_SET);
        if (status < 0) {
            PHYSFS_setErrorCode(PHYSFS_ERR_IO);
            return 0;
        }

        return 1;
    }

    PHYSFS_sint64 tellFunc(PHYSFS_Io* io)
    {
        auto* source = reinterpret_cast<FileHandle*>(io->opaque)->source;

        Sint64 status = SDL_RWtell(source);
        if (status < 0) {
            PHYSFS_setErrorCode(PHYSFS_ERR_IO);
            return -1;
        }

        return status;
    }

    PHYSFS_sint64 sizeFunc(PHYSFS_Io* io)
    {
        auto* handle = reinterpret_cast<FileHandle*>(io->opaque);
        return handle->size;
    }

    PHYSFS_Io* openReadFunc(void* opaque, const char* filename);
    PHYSFS_Io* duplicateFunc(PHYSFS_Io* io)
    {
        auto* handle = reinterpret_cast<FileHandle*>(io->opaque);
        PHYSFS_Io* newIo = openReadFunc(nullptr, handle->filename.data());
        if (!newIo) {
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
            return nullptr;
        }

        return newIo;
    }

    void destroyFunc(PHYSFS_Io* io)
    {
        auto* handle = reinterpret_cast<FileHandle*>(io->opaque);
        auto* source = handle->source;

        SDL_RWclose(source);
        delete handle;
        delete io;
    }

    void* openArchiveFunc(PHYSFS_Io*, const char* name, int forWrite)
    {
        if (forWrite) {
            PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
            return nullptr;
        }

        if (std::string("X.__NX_ASSETS_DIR") != name) {
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
            return nullptr;
        }

        return new char(); // Return pointer to a dummy value ._.
    }

    void enumerateFilesFunc(void* opaque, const char* dirName, PHYSFS_EnumFilesCallback cb,
        const char* origDir, void* callbackData)
    {
        static std::mutex mutex;
        bool attachedThread = false;

        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        if (!env) {
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
            return;
        }

        jstring path = env->NewStringUTF(dirName);
        jstring dirs = (jstring) env->CallStaticObjectMethod(mainClassObj,
            fsListAssetDirectoryMID, path);
        env->DeleteLocalRef(path);

        if (dirs) {
            const char* dirsStr = env->GetStringUTFChars(dirs, 0);
            std::istringstream iss(dirsStr);
            env->ReleaseStringUTFChars(dirs, dirsStr);
            env->DeleteLocalRef(dirs);

            std::string filename;
            while(std::getline(iss, filename, ';')) {
                cb(callbackData, origDir, filename.data());
            }
        }
        else {
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
        }
    }

    PHYSFS_Io* openReadFunc(void* opaque, const char* filename)
    {
        SDL_RWops* source = SDL_RWFromFile(filename, "rb");
        if (!source) return nullptr;

        // PHYSFS Cannot work with files with unknown size
        PHYSFS_sint64 size = SDL_RWsize(source);
        if (size < 0) {
            SDL_RWclose(source);
            return nullptr;
        }

        FileHandle* handle = new FileHandle();
        handle->source = source;
        handle->filename = filename;
        handle->size = size;

        PHYSFS_Io* io = new PHYSFS_Io;
        memset(io, 0, sizeof(PHYSFS_Io));
        io->read = readFunc;
        io->seek = seekFunc;
        io->tell = tellFunc;
        io->length = sizeFunc;
        io->duplicate = duplicateFunc;
        io->destroy = destroyFunc;
        io->opaque = handle;

        return io;
    }

    PHYSFS_Io* unsupportedIOFunc(void*, const char*)
    {
        Log::error("Unsupported IO func");
        PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
        return nullptr;
    }

    int unsupportedIntFunc(void*, const char*)
    {
        Log::error("Unsupported IO func");
        PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
        return 0;
    }

    int statFunc(void* opaque, const char* filename, PHYSFS_Stat* stat)
    {
        stat->readonly = 1;
        stat->createtime = 0;
        stat->modtime = 0;
        stat->accesstime = 0;

        // Try to open file for reading, if can do so, then file is regular
        SDL_RWops* file = SDL_RWFromFile(filename, "rb");
        if (file) {
            stat->filesize = SDL_RWsize(file);
            stat->filetype = PHYSFS_FILETYPE_REGULAR;
            SDL_RWclose(file);
            return 1;
        }

        // Check if directory
        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        if (!env) {
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
            return 0;
        }

        jstring path = env->NewStringUTF(filename);
        jboolean isDir = env->CallStaticBooleanMethod(mainClassObj, fsIsDirectoryMID, path);
        env->DeleteLocalRef(path);

        if (isDir) {
            stat->filesize = -1;
            stat->filetype = PHYSFS_FILETYPE_DIRECTORY;
        }
        else {
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
            return 0;
        }

        return 1;
    }

    void closeArchiveFunc(void* opaque)
    {
        delete static_cast<char*>(opaque);
    }
}
#endif

Filesystem::~Filesystem()
{
    if (PHYSFS_isInit()) PHYSFS_deinit();

    #if defined(NX_SYSTEM_ANDROID)
        delete assetsIO;
        assetsIO = nullptr;
    #endif
}

bool Filesystem::initialize(const char* arg0)
{
    if (!PHYSFS_init(arg0)) return false;

    PHYSFS_permitSymbolicLinks(1);

    // Initialize some Android JNI vars
    #if defined(NX_SYSTEM_ANDROID)

        // Get the environment variable
        JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
        if (!env) {
            Log::fatal("Cannot acquire the JNIEnv from SDL");
            return false;
        }

        // Get the main activity class
        jclass cls = env->FindClass("com/nxsie/m2n/MainActivity");
        if (cls == nullptr) {
            Log::fatal("Error getting MainActivity class object");
            return false;
        }
        mainClassObj = (jclass)env->NewGlobalRef(cls);
        env->DeleteLocalRef(cls);

        // Get the needed static methods
        fsListAssetDirectoryMID = env->GetStaticMethodID(mainClassObj, "fsListAssetDirectory",
            "(Ljava/lang/String;)Ljava/lang/String;");
        if (fsListAssetDirectoryMID == nullptr) {
            Log::fatal("Error getting method id");
            return false;
        }

        fsIsDirectoryMID = env->GetStaticMethodID(mainClassObj, "fsIsDirectory",
            "(Ljava/lang/String;)Z");
        if (fsIsDirectoryMID == nullptr) {
            Log::fatal("Error getting method id for fsIsDirectory");
            return false;
        }

        // Register the android assets "archiver" to Physfs
        PHYSFS_Archiver archiver = {
            0,
            {
                "__NX_ASSETS_DIR",
                "Android assets directory access",
                "Dermoumi S. <sdermoumi@gmail.com>",
                "none",
                1
            },
            openArchiveFunc,
            enumerateFilesFunc,
            openReadFunc,
            unsupportedIOFunc,
            unsupportedIOFunc,
            unsupportedIntFunc,
            unsupportedIntFunc,
            statFunc,
            closeArchiveFunc
        };

        if (!assetsIO) {
            assetsIO = new PHYSFS_Io();
            memset(assetsIO, 0, sizeof(PHYSFS_Io));
            assetsIO->seek = ioSeekFunc;
            assetsIO->tell = ioTellFunc;
            assetsIO->length = ioSizeFunc;
            assetsIO->duplicate = ioDuplicateFunc;
            assetsIO->destroy = ioDestroyFunc;
        }

        if (!PHYSFS_registerArchiver(&archiver)) {
            return false;
        }
    #endif

    return true;
}

bool Filesystem::mountDir(const std::string& dir, const std::string& point, bool append)
{
    return PHYSFS_mount(dir.data(), point.data(), append);
}

bool Filesystem::mountArchive(const std::string& dir, const std::string& point, bool append)
{
    return PHYSFS_mount(dir.data(), point.data(), append);
}

bool Filesystem::mountAssetsDir(const std::string& point, bool append)
{
    #if defined(NX_SYSTEM_ANDROID)
        if(!PHYSFS_mountIo(assetsIO, "X.__NX_ASSETS_DIR", point.data(), append)) {
            Log::error("Cannot mount assets Directory %s", getErrorMessage().data());
            return false;
        }
        return true;

    #else
        std::string assetsDir = getBaseDir() + "assets";
        return PHYSFS_mount(assetsDir.data(), point.data(), append);
    #endif
}

bool Filesystem::setWriteDir(const std::string& dir)
{
    return PHYSFS_setWriteDir(dir.data());
}

std::string Filesystem::getErrorMessage()
{
    const char* message = PHYSFS_getLastError();
    return message ? message : "<unknown error message>";
}

std::string Filesystem::getPrefsDir()
{
    #if defined(NX_SYSTEM_ANDROID)
        const char* dir = SDL_AndroidGetInternalStoragePath();
    #else
        const char* dir = PHYSFS_getPrefDir(GAME_ORGNAME, GAME_SHORTTITLE);
    #endif

    return dir ? dir : "";
}

std::string Filesystem::getBaseDir()
{
    const char* dir = PHYSFS_getBaseDir();
    return dir ? dir : "";
}
