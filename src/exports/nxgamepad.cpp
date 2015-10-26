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
}