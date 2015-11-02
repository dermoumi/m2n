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
#include "../system/log.hpp"
#include "../graphics/glcontext.hpp"

#include <SDL2/SDL.h>
#include <vector>

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxWindow = SDL_Window;

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
namespace
{
    NxWindow* window {nullptr};
}

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------

//------------------------------------------------------
NX_EXPORT NxWindow* nxWindowGet()
{
    return window;
}

//------------------------------------------------------
NX_EXPORT void nxWindowClose()
{
    SDL_DestroyWindow(window);
    window = nullptr;
}

//------------------------------------------------------
NX_EXPORT NxWindow* nxWindowCreate(const char* title, int width, int height, bool fullscreen)
{
    if (window) {
        nxWindowClose();
    }

    Uint32 flags = SDL_WINDOW_OPENGL;

    if (fullscreen) {
        // Add FULLSCREEN_DESKTOP flag is window sizes are the same as the desktop's mode
        SDL_DisplayMode desktopMode;
        if (SDL_GetDesktopDisplayMode(0, &desktopMode) == 0
            && desktopMode.w == width && desktopMode.h == height)
        {
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        // If FULLSCREEN_DESKTOP wasn't added, add regulare FULLSCREEN flag
        if ((flags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, flags);

    auto* context = GlContext::create(0, 0, 0);
    context->setVSyncEnabled(false);

    return window;
}

//------------------------------------------------------
NX_EXPORT void nxWindowDisplay()
{
    GlContext::ensure()->display();
}

//------------------------------------------------------
NX_EXPORT void nxWindowEnsureContext()
{
    GlContext::ensure();
}

//------------------------------------------------------
NX_EXPORT void nxWindowReleaseContext()
{
    if (!Thread::isMain()) GlContext::release();
}

//------------------------------------------------------
NX_EXPORT void nxWindowGetSize(int* sizePtr)
{
    SDL_GetWindowSize(window, &sizePtr[0], &sizePtr[1]);
}

//------------------------------------------------------
NX_EXPORT bool nxWindowGetDesktopSize(int displayIndex, int* sizePtr)
{
    if (displayIndex < 1 || displayIndex > SDL_GetNumVideoDisplays()) return false;

    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(displayIndex - 1, &dm) == 0) {
        sizePtr[0] = dm.w;
        sizePtr[1] = dm.h;
        return true;
    }

    return false;
}

//------------------------------------------------------
NX_EXPORT int nxWindowGetDisplayCount()
{
    return SDL_GetNumVideoDisplays();
}

//------------------------------------------------------
NX_EXPORT const char* nxWindowGetDisplayName(int displayIndex)
{
    return SDL_GetDisplayName(displayIndex);
}

//------------------------------------------------------
NX_EXPORT int nxWindowGetFullscreen()
{
    uint32_t flags = SDL_GetWindowFlags(window);
    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        return 2;
    }
    else if(flags & SDL_WINDOW_FULLSCREEN) {
        return 1;
    }
    else {
        return 0;
    }
}

//------------------------------------------------------
NX_EXPORT const int* nxWindowGetDisplayModes(int displayIndex, size_t* count)
{
    static std::vector<int> modes;

    if (displayIndex < 1 || displayIndex > SDL_GetNumVideoDisplays()) {
        *count = 0;
        return modes.data();
    }

    size_t displayCount = SDL_GetNumDisplayModes(displayIndex);
    modes.resize(displayCount * 2);
    for (size_t i = 0u; i < displayCount; ++i) {
        SDL_DisplayMode dm;
        if (SDL_GetDisplayMode(displayIndex, i, &dm) != 0) {
            modes.clear();
            break;
        }

        modes[i]     = dm.w;
        modes[i + 1] = dm.h;
    }

    *count = modes.size();
    return modes.data();
}

//------------------------------------------------------
NX_EXPORT void nxWindowGetPosition(int* posPtr)
{
    SDL_GetWindowPosition(window, &posPtr[0], &posPtr[1]);
}

//------------------------------------------------------
NX_EXPORT const char* nxWindowGetTitle()
{
    return SDL_GetWindowTitle(window);
}

//------------------------------------------------------
NX_EXPORT void nxWindowMinimize()
{
    SDL_MinimizeWindow(window);
}

//------------------------------------------------------
NX_EXPORT void nxWindowSetPosition(int x, int y)
{
    SDL_SetWindowPosition(window, x, y);
}

//------------------------------------------------------
NX_EXPORT void nxWindowSetSize(int width, int height)
{
    SDL_SetWindowSize(window, width, height);
}

//------------------------------------------------------
NX_EXPORT void nxWindowSetTitle(const char* title)
{
    SDL_SetWindowTitle(window, title);
}

//------------------------------------------------------
NX_EXPORT void nxWindowSimpleMessageBox(const char* title, const char* message, uint32_t type,
    bool attach)
{
    SDL_ShowSimpleMessageBox(type, title, message, attach ? window : nullptr);
}

//------------------------------------------------------
NX_EXPORT void nxWindowGetDrawableSize(int* sizePtr)
{
    SDL_GL_GetDrawableSize(window, &sizePtr[0], &sizePtr[1]);
}

//==============================================================================
