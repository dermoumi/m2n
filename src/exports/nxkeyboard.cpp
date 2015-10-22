#include "../config.hpp"
#include <SDL2/SDL.h>

extern "C"
{    
    NX_EXPORT uint32_t nxKeyboardToKeysym(uint32_t scancode)
    {
        return SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(scancode));
    }

    NX_EXPORT uint32_t nxKeyboardToScancode(uint32_t keysym)
    {
        return SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(keysym));
    }

    NX_EXPORT bool nxKeyboardModKeyDown(uint16_t mod)
    {
        return (SDL_GetModState() & mod);
    }

    NX_EXPORT void nxKeyboardStartTextInput(int x, int y, int w, int h)
    {
        SDL_Rect rect {x, y, w, h};
        SDL_SetTextInputRect(&rect);
        SDL_StartTextInput();
    }

    NX_EXPORT void nxKeyboardStopTextInput()
    {
        SDL_StopTextInput();
    }

    NX_EXPORT bool nxKeyboardTextInputActive()
    {
        return SDL_IsTextInputActive() == SDL_TRUE;
    }
}