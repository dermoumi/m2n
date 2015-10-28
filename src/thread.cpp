#include "thread.hpp"

namespace
{
    thread_local bool isMainThread = false;
}

namespace Thread
{
    void setMain()
    {
        isMainThread = true;
    }

    bool isMain()
    {
        return isMainThread;
    }
}
