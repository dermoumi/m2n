#include "filesystem.hpp"
#include <physfs/physfs.h>

Filesystem::~Filesystem()
{
    if (PHYSFS_isInit()) PHYSFS_deinit();
}

bool Filesystem::initialize(const char* arg0)
{
    if (!PHYSFS_init(arg0)) return false;

    PHYSFS_permitSymbolicLinks(1);

    return true;
}

bool Filesystem::mountDir(const std::string& dir, const std::string& point, bool append)
{
    return PHYSFS_mount(dir.data(), point.data(), append);
}

bool Filesystem::mountRes(const std::string& dir, const std::string& point, bool append)
{
    return PHYSFS_mount(dir.data(), point.data(), append);
}

bool Filesystem::mountArchive(const std::string& dir, const std::string& point, bool append)
{
    return PHYSFS_mount(dir.data(), point.data(), append);
}

bool Filesystem::setWriteDir(const std::string& dir)
{
    return PHYSFS_setWriteDir(dir.data());
}

std::string Filesystem::getErrorMessage()
{
    thread_local std::string errorMessage;

    const char* message = PHYSFS_getLastError();
    errorMessage = message ? message : "";

    return errorMessage;
}

std::string Filesystem::getPrefsDir()
{
    thread_local std::string path;

    const char* dir = PHYSFS_getPrefDir(GAME_ORGNAME, GAME_SHORTTITLE);
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