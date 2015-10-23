#include "../config.hpp"
#include <SDL2/SDL.h>
#include "../log.hpp"

namespace
{
    int currentCursor = -1;
}

extern "C"
{
    NX_EXPORT bool nxMouseCursorVisible(int toggle)
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
}
