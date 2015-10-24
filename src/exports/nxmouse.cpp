#include "../config.hpp"
#include <SDL2/SDL.h>
#include "../log.hpp"

namespace
{
    int currentCursor = -1;
    // bool grabbed = false;
}

extern "C"
{
    SDL_Window* nxWindowGet();

    NX_EXPORT bool nxMouseVisible(int toggle)
    {
        return SDL_ShowCursor(toggle) == 1;
    }

    NX_EXPORT void nxMouseSetSystemCursor(int type)
    {
        SDL_Cursor* cursor;
        if (type == -1) {
            Log::info("default cursor on");
            cursor = SDL_GetDefaultCursor();
        }
        else {
            cursor = SDL_CreateSystemCursor(static_cast<SDL_SystemCursor>(type));
        }

        SDL_SetCursor(cursor);
        currentCursor = type;
    }

    NX_EXPORT int nxMouseGetCursor()
    {
        return currentCursor;
    }

    NX_EXPORT void nxMouseSetPosition(int x, int y, bool global)
    {
        if (global) {
            // TODO: Upgrade to SDL 2.0.4?
            // SDL_WarpMouseGlobal(x, y);
        }
        else {
            SDL_WarpMouseInWindow(nxWindowGet(), x, y);
        }
    }

    NX_EXPORT void nxMouseGetPosition(int* pos, bool global)
    {
        if (SDL_GetRelativeMouseMode()) {
            SDL_GetRelativeMouseState(&pos[0], &pos[1]);
        }
        else if (global) {
            // TODO: Upgrade to SDL 2.0.4?
            // SDL_GetGlobalMouseState(&pos[0], &pos[1]);
        }
        else {
            SDL_GetMouseState(&pos[0], &pos[1]);
        }
    }

    NX_EXPORT bool nxMouseSetRelativeMode(bool enabled)
    {
        return SDL_SetRelativeMouseMode(static_cast<SDL_bool>(enabled)) == 0;
    }

    NX_EXPORT bool nxMouseGetRelativeMode()
    {
        return SDL_GetRelativeMouseMode() == SDL_TRUE;
    }

    NX_EXPORT bool nxMouseIsButtonDown(int button)
    {
        return SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(button);
    }

    NX_EXPORT bool nxMouseSetGrab(bool /*enabled*/)
    {
        // TODO: Upgrade to SDL 2.0.4?
        // if (SDL_CaptureMouse(enabled) == 0) {
        //     grabbed = enabled;
        //     return true;
        // }

        return false;
    }

    NX_EXPORT bool nxMouseIsGrabbed()
    {
        // TODO: Upgrade to SDL 2.0.4?
        // return grabbed;
        return false;
    }

}
