#include "../config.hpp"
#include <thread>
#include <luajit/lua.hpp>

namespace
{
    struct NxThreadObj
    {
        std::thread* handle;
        lua_State* state;
        bool succeeded;
    };

    void threadCallback(lua_State* state, bool& succeeded)
    {
        int argCount = lua_gettop(state) - 1; // Total elements in stack minus the function itself
        printf("Arg count: %i\n", argCount);
        succeeded = lua_pcall(state, argCount, -1, 0) == 0;
    }
}

extern "C"
{
    NX_EXPORT NxThreadObj* nxThreadCreate(lua_State* state)
    {
        auto threadObj = new NxThreadObj();
        threadObj->state = state;
        threadObj->handle = new std::thread(threadCallback, state, std::ref(threadObj->succeeded));
        return threadObj;
    }

    NX_EXPORT void nxThreadRelease(NxThreadObj* threadObj)
    {
        delete threadObj->handle;
        delete threadObj;
    }

    NX_EXPORT bool nxThreadWait(NxThreadObj* threadObj)
    {
        threadObj->handle->join();
        return threadObj->succeeded;
    }
}