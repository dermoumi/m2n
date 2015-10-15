#include "../config.hpp"
#include <SDL2/SDL.h>

using NxWindow = SDL_Window;

namespace
{
    NxWindow* window {nullptr};
}

extern "C"
{
    NX_EXPORT NxWindow* nxWindowGet()
    {
        return window;
    }

    NX_EXPORT void nxWindowClose(NxWindow* window)
    {
        SDL_DestroyWindow(window);
    }

    NX_EXPORT NxWindow* nxWindowCreate(const char* title, int width, int height, bool fullscreen)
    {
        if (window) {
            nxWindowClose(window);
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

        return window;
    }
}