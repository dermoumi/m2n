#include "../config.hpp"
#include <mutex>

using NxMutex = std::mutex;

extern "C"
{
    NX_EXPORT NxMutex* nxMutexCreate()
    {
        return new std::mutex();
    }

    NX_EXPORT void nxMutexRelease(NxMutex* mutex)
    {
        delete mutex;
    }

    NX_EXPORT void nxMutexLock(NxMutex* mutex)
    {
        mutex->lock();
    }

    NX_EXPORT bool nxMutexTryLock(NxMutex* mutex)
    {
        return mutex->try_lock();
    }

    NX_EXPORT void nxMutexUnlock(NxMutex* mutex)
    {
        mutex->unlock();
    }
}
