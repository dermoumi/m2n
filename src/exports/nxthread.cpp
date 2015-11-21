/*//============================================================================
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
*///============================================================================
#include "../config.hpp"
#include "../system/thread.hpp"

#include <luajit/lua.hpp>
#include <thread>

//----------------------------------------------------------
// Locals
//----------------------------------------------------------

struct NxThreadObj
{
    std::thread* handle;
    lua_State* state;
    bool ownsState;
    bool succeeded;
};

static void threadCallback(NxThreadObj* thread)
{
    int argCount = lua_gettop(thread->state) - 1; // Total elements in stack minus the function itself
    thread->succeeded = lua_pcall(thread->state, argCount, -1, 0) == 0;

    if (thread->ownsState) {
        lua_close(thread->state);
        delete thread->handle;
        delete thread;
    }
}

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT NxThreadObj* nxThreadCreate(lua_State* state)
{
    auto threadObj = new NxThreadObj();
    threadObj->state = state;
    threadObj->ownsState = false;
    threadObj->handle = new std::thread(threadCallback, threadObj);
    return threadObj;
}

//----------------------------------------------------------
NX_EXPORT void nxThreadRelease(NxThreadObj* threadObj)
{
    delete threadObj->handle;
    delete threadObj;
}

//----------------------------------------------------------
NX_EXPORT bool nxThreadWait(NxThreadObj* threadObj)
{
    threadObj->handle->join();
    return threadObj->succeeded;
}

//----------------------------------------------------------
NX_EXPORT void nxThreadDetach(NxThreadObj* threadObj)
{
    threadObj->ownsState = true;
    threadObj->handle->detach();
}

//----------------------------------------------------------
NX_EXPORT bool nxThreadIsMain()
{
    return Thread::isMain();
}

//==============================================================================
