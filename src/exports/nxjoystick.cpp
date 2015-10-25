#include "../config.hpp"
#include <SDL2/SDL.h>

using NxJoystick = SDL_Joystick;

extern "C"
{
    NX_EXPORT NxJoystick* nxJoystickOpen(int id)
    {
        return SDL_JoystickOpen(id - 1);
    }

    NX_EXPORT void nxJoystickClose(NxJoystick* joystick)
    {
        SDL_JoystickClose(joystick);
    }
}