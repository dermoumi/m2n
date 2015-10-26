#include "../config.hpp"
#include <SDL2/SDL.h>

using NxGamepad = SDL_GameController;

extern "C"
{
    NX_EXPORT NxGamepad* nxGamepadOpen(int id)
    {
        return SDL_GameControllerOpen(id - 1);
    }

    NX_EXPORT void nxGamepadClose(NxGamepad* gamepad)
    {
        SDL_GameControllerClose(gamepad);
    }

    NX_EXPORT bool nxGamepadButtonDown(NxGamepad* gamepad, int btn)
    {
        return SDL_GameControllerGetButton(gamepad, static_cast<SDL_GameControllerButton>(btn - 1));
    }

    NX_EXPORT double nxGamepadGetAxis(NxGamepad* gamepad, int axis)
    {
        return SDL_GameControllerGetAxis(gamepad, static_cast<SDL_GameControllerAxis>(axis - 1));
    }
}