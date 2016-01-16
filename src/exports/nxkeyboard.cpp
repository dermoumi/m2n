/*
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
*/

#include "../config.hpp"

#include <SDL2/SDL.h>

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
    return (SDL_GetModState() & mod) != 0;
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
