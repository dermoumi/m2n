#include "filesystem.hpp"
#include "log.hpp"
#include <sstream>
#include <mutex>
#include <physfs/physfs.h>
#include <SDL2/SDL.h>

#if defined(NX_SYSTEM_ANDROID)
#include <jni.h>
namespace
{
    static PHYSFS_Io* assetsIO = nullptr;

    static JavaVM* jvm = nullptr;
    static jclass mainClassObj = nullptr;
    static jmethodID fsListAssetDirectoryMID = nullptr;

    PHYSFS_sint64 ioReadFunc(PHYSFS_Io*, void*, PHYSFS_uint64)
    {
        Log::info("Read func?");
        return 0;
    }

    int ioSeekFunc(PHYSFS_Io*, PHYSFS_uint64)
    {
        Log::info("Seek func?");
        return 1;
    }

    PHYSFS_sint64 ioTellFunc(PHYSFS_Io*)
    {
        Log::info("Tell func?");
        return 0;
    }

    PHYSFS_sint64 ioSizeFunc(PHYSFS_Io*)
    {
        Log::info("Size func?");
        return 0;
    }

    PHYSFS_Io* ioDuplicateFunc(PHYSFS_Io*)
    {
        Log::info("Duplicate func?");
        return nullptr;
    }

    void ioDestroyFunc(PHYSFS_Io*)
    {
        Log::info("Destroy func");
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
        Log::info("Enumerate files func");
        if (forWrite) {
            PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
            return nullptr;
        }

        Log::info("info3");
        if (std::string("X.__NX_ASSETS_DIR") != name) {
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
            return nullptr;
        }

        Log::info("Mounting assets dir :)");
        return new char(); // Return pointer to a dummy value ._.
    }

    void enumerateFilesFunc(void* opaque, const char* dirName, PHYSFS_EnumFilesCallback cb,
        const char* origDir, void* callbackData)
    {
        Log::info("Enumerate files func");
        static std::mutex mutex;

        Log::info("test1 %s", dirName);
        JNIEnv* env;
        {
            std::lock_guard<std::mutex> lock_guard(mutex);
            jint rs = jvm->AttachCurrentThread(&env, nullptr);
            if (rs != JNI_OK) {
                PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
                return;
            }
        }
        Log::info("test2");

        jstring path = env->NewStringUTF(dirName);
        jstring dirs = (jstring) env->CallStaticObjectMethod(mainClassObj,
            fsListAssetDirectoryMID, path);
        env->DeleteLocalRef(path);

        Log::info("test3");
        if (dirs) {
            Log::info("test4");
            const char* dirsStr = env->GetStringUTFChars(dirs, 0);
            Log::info("test5 %s", dirsStr);
            std::istringstream iss(dirsStr);
            env->ReleaseStringUTFChars(dirs, dirsStr);
            env->DeleteLocalRef(dirs);

            std::string filename;
            while(std::getline(iss, filename, ';')) {
                Log::info("File %s", filename.data());
                cb(callbackData, origDir, filename.data());
            }
        }
        else {
            Log::info("test6");
            PHYSFS_setErrorCode(PHYSFS_ERR_OTHER_ERROR);
        }

        {
            std::lock_guard<std::mutex> lock_guard(mutex);
            jvm->DetachCurrentThread();
        }
    }

    PHYSFS_Io* openReadFunc(void* opaque, const char* filename)
    {
        Log::info("Open read func");
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
        Log::info("Unsupported IO func");
        PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
        return nullptr;
    }

    int unsupportedIntFunc(void*, const char*)
    {
        Log::info("Unsupported int func");
        PHYSFS_setErrorCode(PHYSFS_ERR_READ_ONLY);
        return 0;
    }

    int statFunc(void* opaque, const char* fn, PHYSFS_Stat* stat)
    {
        Log::info("statFunc");

        stat->readonly = 1;
        stat->createtime = 0;
        stat->modtime = 0;
        stat->accesstime = 0;
        stat->filesize = 0;
        stat->filetype = PHYSFS_FILETYPE_DIRECTORY;

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

        // Get the Java VM
        jint rs = env->GetJavaVM(&jvm);
        if (rs != JNI_OK) {
            Log::fatal("cannot acquire the Java VM");
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
            assetsIO->read = ioReadFunc;
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
        Log::info("aHello?");
        if(!PHYSFS_mountIo(assetsIO, "X.__NX_ASSETS_DIR", point.data(), append)) {
            Log::error("thing %s", getErrorMessage().data());
            return false;
        }
        return true;
        Log::info("aHello2");

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
    thread_local std::string errorMessage;

    const char* message = PHYSFS_getLastError();
    errorMessage = message ? message : "<unknown error message>";

    return errorMessage;
}

std::string Filesystem::getPrefsDir()
{
    thread_local std::string path;

    #if defined(NX_SYSTEM_ANDROID)
        const char* dir = SDL_AndroidGetInternalStoragePath();
    #else
        const char* dir = PHYSFS_getPrefDir(GAME_ORGNAME, GAME_SHORTTITLE);
    #endif

    path = dir ? dir : "";

    return path;
}

std::string Filesystem::getBaseDir()
{
    thread_local std::string path;

    const char* dir = PHYSFS_getBaseDir();
    path = dir ? dir : "";

    return path;
}