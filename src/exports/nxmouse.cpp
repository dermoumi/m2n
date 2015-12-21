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
#include "../system/log.hpp"

#include <SDL2/SDL.h>

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
// static bool grabbed = false;

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
extern "C" SDL_Window* nxWindowGet();

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT bool nxMouseVisible(int toggle)
{
    return SDL_ShowCursor(toggle) == 1;
}

//----------------------------------------------------------
NX_EXPORT void nxMouseSetSystemCursor(int type)
{
    static SDL_Cursor* cursor;
    if (type == -1) {
        cursor = SDL_GetDefaultCursor();
    }
    else {
        cursor = SDL_CreateSystemCursor(static_cast<SDL_SystemCursor>(type));
    }

    SDL_SetCursor(cursor);
}

//----------------------------------------------------------
NX_EXPORT void nxMouseSetImageCursor(const void* data, int width, int height, int originX,
    int originY)
{
    uint32_t rmask, gmask, bmask, amask;
 
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
    #else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
    #endif
 
    static SDL_Surface* surface;
    surface = SDL_CreateRGBSurfaceFrom(
        const_cast<void*>(data), width, height, 32, width * 4, rmask, gmask, bmask, amask
    );
    
    if (!surface) {
        Log::error("Cursor surface creation failde: %s", SDL_GetError());
        return;
    }
 
    static SDL_Cursor* cursor;
    cursor = SDL_CreateColorCursor(surface, originX, originY);
    // SDL_FreeSurface(surface);

    SDL_SetCursor(cursor);
}

//----------------------------------------------------------
NX_EXPORT void nxMouseSetPosition(int x, int y, bool global)
{
    if (global) {
        // Upgrade to SDL 2.0.4?
        // SDL_WarpMouseGlobal(x, y);
    }
    else {
        SDL_WarpMouseInWindow(nxWindowGet(), x, y);
    }
}

//----------------------------------------------------------
NX_EXPORT void nxMouseGetPosition(int* pos, bool global)
{
    if (SDL_GetRelativeMouseMode()) {
        SDL_GetRelativeMouseState(&pos[0], &pos[1]);
    }
    else if (global) {
        // Upgrade to SDL 2.0.4?
        // SDL_GetGlobalMouseState(&pos[0], &pos[1]);
    }
    else {
        SDL_GetMouseState(&pos[0], &pos[1]);
    }
}

//----------------------------------------------------------
NX_EXPORT bool nxMouseSetRelativeMode(bool enabled)
{
    return SDL_SetRelativeMouseMode(static_cast<SDL_bool>(enabled)) == 0;
}

//----------------------------------------------------------
NX_EXPORT bool nxMouseGetRelativeMode()
{
    return SDL_GetRelativeMouseMode() == SDL_TRUE;
}

//----------------------------------------------------------
NX_EXPORT bool nxMouseIsButtonDown(int button)
{
    return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(button);
}

//----------------------------------------------------------
NX_EXPORT bool nxMouseSetGrab(bool /*enabled*/)
{
    // Upgrade to SDL 2.0.4?
    // if (SDL_CaptureMouse(enabled) == 0) {
    //     grabbed = enabled;
    //     return true;
    // }

    return false;
}

//----------------------------------------------------------
NX_EXPORT bool nxMouseIsGrabbed()
{
    // Upgrade to SDL 2.0.4?
    // return grabbed;
    return false;
}

//==============================================================================
