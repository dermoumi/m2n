#include "../config.hpp"
#include <SDL2/SDL.h>
#include <string>

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

    NX_EXPORT int nxJoystickButtonCount(NxJoystick* joystick)
    {
        return SDL_JoystickNumButtons(joystick);
    }

    NX_EXPORT int nxJoystickAxisCount(NxJoystick* joystick)
    {
        return SDL_JoystickNumAxes(joystick);
    }

    NX_EXPORT int nxJoystickBallCount(NxJoystick* joystick)
    {
        return SDL_JoystickNumBalls(joystick);
    }

    NX_EXPORT int nxJoystickHatCount(NxJoystick* joystick)
    {
        return SDL_JoystickNumHats(joystick);
    }

    NX_EXPORT bool nxJoystickGetButton(NxJoystick* joystick, int button)
    {
        return SDL_JoystickGetButton(joystick, button - 1);
    }

    NX_EXPORT double nxJoystickGetAxis(NxJoystick* joystick, int axis)
    {
        return SDL_JoystickGetAxis(joystick, axis - 1);
    }

    NX_EXPORT bool nxJoystickGetBall(NxJoystick* joystick, int ball, int* position)
    {
        return SDL_JoystickGetBall(joystick, ball - 1, &position[0], &position[1]);
    }

    NX_EXPORT uint8_t nxJoystickGetHat(NxJoystick* joystick, int hat)
    {
        return SDL_JoystickGetHat(joystick, hat - 1);
    }

    NX_EXPORT const char* nxJoystickGetName(NxJoystick* joystick)
    {
        return SDL_JoystickName(joystick);
    }

    NX_EXPORT const char* nxJoystickGetGUID(NxJoystick* joystick)
    {
        static std::string guidStr;

        guidStr.resize(64);
        auto guid = SDL_JoystickGetGUID(joystick);
        SDL_JoystickGetGUIDString(guid, &guidStr[0], guidStr.size());

        return guidStr.data();
    }
}