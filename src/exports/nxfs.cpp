#include "../config.hpp"
#include <physfs/physfs.h>

extern "C"
{
    NX_EXPORT char** nxFsEnumerateFiles(const char* path)
    {
        return PHYSFS_enumerateFiles(path);
    }

    NX_EXPORT void nxFsFreeList(void* listVar)
    {
        PHYSFS_freeList(listVar);
    }

    NX_EXPORT bool nxFsIsDirectory(const char* path)
    {
        PHYSFS_Stat stat;
        if (!PHYSFS_stat(path, &stat)) return false;

        return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
    }

    NX_EXPORT bool nxFsIsFile(const char* path)
    {
        PHYSFS_Stat stat;
        if (!PHYSFS_stat(path, &stat)) return false;

        return stat.filetype == PHYSFS_FILETYPE_REGULAR;
    }
}