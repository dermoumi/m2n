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

#include <SDL2/SDL.h>
#include <string>
#include <thread>
#include <chrono>

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
extern "C"
{
    //------------------------------------------------------
    NX_EXPORT void nxSysSleep(double s)
    {
        auto time = std::chrono::duration<double>(s);
        std::this_thread::sleep_for(time);
    }

    //------------------------------------------------------
    NX_EXPORT double nxSysGetTime()
    {
        auto t = std::chrono::high_resolution_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(t).count() / 1000.0;
    }

    //------------------------------------------------------
    NX_EXPORT const char* nxSysGetSDLError()
    {
        thread_local std::string errorMessage;
        errorMessage = SDL_GetError();
        return errorMessage.data();
    }

    //------------------------------------------------------
    NX_EXPORT const char* nxSysGetPlatform()
    {
        return SDL_GetPlatform();
    }
}

//==============================================================================
