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
#include "../graphics/image.hpp"

#include <SDL2/SDL.h>
#include <vector>

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxWindow = SDL_Window;

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
static NxWindow* window {nullptr};
static Image icon;

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT NxWindow* nxWindowGet()
{
    return window;
}

//----------------------------------------------------------
NX_EXPORT void nxWindowClose()
{
    if (!window) return;
    
    GlContext::release();
    SDL_DestroyWindow(window);
    window = nullptr;
}

//----------------------------------------------------------
NX_EXPORT NxWindow* nxWindowCreate(const char* title, int width, int height, int fullscreen,
    int display, bool vsync, bool resizable, bool borderless, int minWidth, int minHeight,
    bool highDpi, int refreshRate, int posX, int posY, int depthBits, int stencilBits, int msaa)
{
    // Get a valid display number
    display = std::max(0, std::min(display - 1, SDL_GetNumVideoDisplays() - 1));



    if      (posX == -1) posX = SDL_WINDOWPOS_UNDEFINED_DISPLAY(display);
    else if (posX == -2) posX = SDL_WINDOWPOS_CENTERED_DISPLAY(display);

    if      (posY == -1) posY = SDL_WINDOWPOS_UNDEFINED_DISPLAY(display);
    else if (posY == -2) posY = SDL_WINDOWPOS_CENTERED_DISPLAY(display);

    // Fullscreen
    uint32_t fullscreenFlags = 0;
    if (fullscreen == 3) {
        // Add FULLSCREEN_DESKTOP flag is window sizes are the same as the desktop's mode
        SDL_DisplayMode dm;
        if (SDL_GetDesktopDisplayMode(display, &dm) == 0 && dm.w == width && dm.h == height) {
            fullscreenFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }

        // If FULLSCREEN_DESKTOP wasn't added, add regulare FULLSCREEN flag
        if ((fullscreenFlags & SDL_WINDOW_FULLSCREEN_DESKTOP) == 0) {
            fullscreenFlags |= SDL_WINDOW_FULLSCREEN;
        }
    }
    else if (fullscreen == 2) {
        fullscreenFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_FULLSCREEN;
    }
    else if (fullscreen == 1) {
        fullscreenFlags |= SDL_WINDOW_FULLSCREEN;
    }

    if (!window) {
        // No window yet, set up flags and create a new window
        uint32_t flags = SDL_WINDOW_OPENGL | fullscreenFlags;

        if (resizable)  flags |= SDL_WINDOW_RESIZABLE;
        if (borderless) flags |= SDL_WINDOW_BORDERLESS;
        if (highDpi)    flags |= SDL_WINDOW_ALLOW_HIGHDPI;

        // Set context attributes    
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   depthBits);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencilBits);

        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaa ? 1 : 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, NX_GL_MAJOR);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, NX_GL_MINOR);

        #ifdef NX_OPENGL_ES
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        #endif

        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

        window = SDL_CreateWindow(title, posX, posY, width, height, flags);
    }
    else {
        // There's already a window
        // try to change whatever settings we can and leave it at that
        SDL_SetWindowTitle(window, title);
        SDL_SetWindowPosition(window, posX, posY);
        SDL_SetWindowSize(window, width, height);

        SDL_SetWindowBordered(window, borderless ? SDL_FALSE : SDL_TRUE);

        // Set display mode
        SDL_DisplayMode target, closest;
        target.w = width;
        target.h = height;
        target.refresh_rate = refreshRate;
        target.format = 0;
        target.driverdata = 0;
        if (SDL_GetClosestDisplayMode(display, &target, &closest) != nullptr) {
            SDL_SetWindowDisplayMode(window, &closest);
        }
        SDL_SetWindowFullscreen(window, fullscreenFlags);
    }

    SDL_SetWindowMinimumSize(window, minWidth, minHeight);

    // Context settings
    auto* context = GlContext::ensure();
    context->setVSyncEnabled(vsync);

    return window;
}

//----------------------------------------------------------
NX_EXPORT void nxWindowDisplay()
{
    GlContext::ensure()->display();
}

//----------------------------------------------------------
NX_EXPORT void nxWindowGetFlags(int* flagsPtr)
{
    auto flags = SDL_GetWindowFlags(window);

    // Get fullscreen
    if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        flagsPtr[0] = 2;
    }
    else if (flags & SDL_WINDOW_FULLSCREEN) {
        flagsPtr[0] = 1;
    }
    else {
        flagsPtr[0] = 0;
    }

    // Display
    flagsPtr[1] = std::max(0, SDL_GetWindowDisplayIndex(window));

    // Vsync
    flagsPtr[2] = GlContext::ensure()->isVSyncEnabled() ? 1 : 0;

    // Resizable
    flagsPtr[3] = (flags & SDL_WINDOW_RESIZABLE) ? 1 : 0;

    // Borderless
    flagsPtr[4] = (flags & SDL_WINDOW_BORDERLESS) ? 1 : 0;

    // Minimum width and height
    SDL_GetWindowMinimumSize(window, &flagsPtr[5], &flagsPtr[6]);

    // High DPI
    flagsPtr[7] = (flags & SDL_WINDOW_ALLOW_HIGHDPI);

    // Refresh rate
    SDL_DisplayMode dm;
    flagsPtr[8] = (SDL_GetWindowDisplayMode(window, &dm) == 0) ? dm.refresh_rate : 0;

    // Position
    SDL_GetWindowPosition(window, &flagsPtr[9], &flagsPtr[10]);

    // Depth and stencil bits
    if (SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE, &flagsPtr[11]) != 0) flagsPtr[11] = 0;
    if (SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE, &flagsPtr[12]) != 0) flagsPtr[12] = 0;

    // MSAA
    int msaaEnabled = 0;
    if (SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msaaEnabled) != 0) msaaEnabled = 0;

    if (!msaaEnabled || SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &flagsPtr[13]) != 0) {
        flagsPtr[13] = 0;
    }
}

//----------------------------------------------------------
NX_EXPORT void nxWindowEnsureContext()
{
    GlContext::ensure();
}

//----------------------------------------------------------
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

//----------------------------------------------------------
NX_EXPORT int nxWindowGetDisplayCount()
{
    return SDL_GetNumVideoDisplays();
}

//----------------------------------------------------------
NX_EXPORT const char* nxWindowGetDisplayName(int displayIndex)
{
    if (displayIndex < 1 || displayIndex > SDL_GetNumVideoDisplays()) return nullptr;

    return SDL_GetDisplayName(displayIndex - 1);
}

//----------------------------------------------------------
NX_EXPORT int nxWindowGetFullscreen()
{
    auto flags = SDL_GetWindowFlags(window);
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

//----------------------------------------------------------
NX_EXPORT bool nxWindowGetVisible()
{
    auto flags = SDL_GetWindowFlags(window);
    return (flags & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED));
}

//----------------------------------------------------------
NX_EXPORT const int* nxWindowGetDisplayModes(int displayIndex, size_t* count)
{
    static std::vector<int> modes;

    if (displayIndex < 1 || displayIndex > SDL_GetNumVideoDisplays()) {
        modes.clear();
    }
    else {
        size_t displayCount = SDL_GetNumDisplayModes(displayIndex - 1);
        modes.resize(displayCount * 2);
        for (size_t i = 0u; i < displayCount; ++i) {
            SDL_DisplayMode dm;
            if (SDL_GetDisplayMode(displayIndex - 1, i, &dm) != 0) {
                modes.clear();
                break;
            }

            modes[i*2]     = dm.w;
            modes[i*2 + 1] = dm.h;
        }
    }

    *count = modes.size();
    return modes.data();
}

//----------------------------------------------------------
NX_EXPORT void nxWindowGetSize(int* sizePtr)
{   
    SDL_GetWindowSize(window, &sizePtr[0], &sizePtr[1]);
}

//----------------------------------------------------------
NX_EXPORT void nxWindowGetPosition(int* posPtr)
{
    SDL_GetWindowPosition(window, &posPtr[0], &posPtr[1]);
}

//----------------------------------------------------------
NX_EXPORT const char* nxWindowGetTitle()
{
    return SDL_GetWindowTitle(window);
}

//----------------------------------------------------------
NX_EXPORT void nxWindowMinimize()
{
    SDL_MinimizeWindow(window);
}

//----------------------------------------------------------
NX_EXPORT void nxWindowSetPosition(int x, int y)
{
    SDL_SetWindowPosition(window, x, y);
}

//----------------------------------------------------------
NX_EXPORT void nxWindowSetSize(int width, int height)
{
    SDL_SetWindowSize(window, width, height);
}

//----------------------------------------------------------
NX_EXPORT void nxWindowSetTitle(const char* title)
{
    SDL_SetWindowTitle(window, title);
}

//----------------------------------------------------------
NX_EXPORT void nxWindowSimpleMessageBox(const char* title, const char* message, uint32_t type,
    bool attach)
{
    SDL_ShowSimpleMessageBox(type, title, message, attach ? window : nullptr);
}

//----------------------------------------------------------
NX_EXPORT int nxWindowMessageBox(const char* title, const char* message, const char** entries,
    uint32_t count, uint32_t accept, uint32_t cancel, uint32_t type, bool attach)
{
    std::vector<SDL_MessageBoxButtonData> buttons(count);
    for (uint32_t i = 0; i < count; ++i) {
        buttons[i].flags = 0;

        if (i+1 == accept) {
            buttons[i].flags |= SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
        }
        if (i+1 == cancel) {
            buttons[i].flags |= SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
        }

        buttons[i].buttonid = i;
        buttons[i].text = entries[i];
    }

    const SDL_MessageBoxData messageboxdata {
        type,
        attach ? window : nullptr,
        title,
        message,
        static_cast<int>(count),
        buttons.data(),
        nullptr
    };

    int buttonID;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonID) < 0) {
        Log::error("Error displaying message box: %s", SDL_GetError());
        return -1;
    }

    return buttonID + 1;
}

//----------------------------------------------------------
NX_EXPORT void nxWindowGetDrawableSize(int* sizePtr)
{
    SDL_GL_GetDrawableSize(window, &sizePtr[0], &sizePtr[1]);
}

//----------------------------------------------------------
NX_EXPORT const uint8_t* nxWindowGetIcon(unsigned int* sizePtr)
{
    icon.getSize(&sizePtr[0], &sizePtr[1]);
    return icon.getPixelsPtr();
}

//----------------------------------------------------------
NX_EXPORT void nxWindowSetIcon(unsigned int width, unsigned int height, const uint8_t* data)
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
 
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
        const_cast<uint8_t*>(data), width, height, 32, width * 4, rmask, gmask, bmask, amask
    );

    if (!surface) {
        Log::error("Window's icon surface creation failed: %s", SDL_GetError());
        return;
    }
 
    SDL_SetWindowIcon(window, surface);
    SDL_FreeSurface(surface);

    icon.create(width, height, data);
}

//==============================================================================
