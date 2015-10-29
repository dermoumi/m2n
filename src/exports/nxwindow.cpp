#include "../config.hpp"
#include "../glcontext.hpp"
#include "../thread.hpp"
#include <SDL2/SDL.h>


#if defined(NX_OPENGL_ES)
    #include <SDL2/SDL_opengles2.h>
#else
    #include <SDL2/SDL_opengl.h>
#endif

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

    NX_EXPORT void nxWindowClose()
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

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

        GlContext::create(0, 0, 0);

        return window;
    }

    NX_EXPORT void nxWindowClear()
    {
        glClearColor(1, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    NX_EXPORT void nxWindowDisplay()
    {
        GlContext::ensure()->display();
    }

    NX_EXPORT void nxWindowEnsureContext()
    {
        GlContext::ensure();
    }

    NX_EXPORT void nxWindowReleaseContext()
    {
        if (!Thread::isMain()) GlContext::release();
    }
}