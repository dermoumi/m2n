/*//============================================================================
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
*///============================================================================
#include "../config.hpp"

#include <SDL2/SDL.h>
#include <string>

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxJoystick = SDL_Joystick;

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
extern "C"
{
    //------------------------------------------------------
    NX_EXPORT NxJoystick* nxJoystickOpen(int id)
    {
        return SDL_JoystickOpen(id - 1);
    }

    //------------------------------------------------------
    NX_EXPORT void nxJoystickClose(NxJoystick* joystick)
    {
        SDL_JoystickClose(joystick);
    }

    //------------------------------------------------------
    NX_EXPORT int nxJoystickButtonCount(NxJoystick* joystick)
    {
        return SDL_JoystickNumButtons(joystick);
    }

    //------------------------------------------------------
    NX_EXPORT int nxJoystickAxisCount(NxJoystick* joystick)
    {
        return SDL_JoystickNumAxes(joystick);
    }

    //------------------------------------------------------
    NX_EXPORT int nxJoystickBallCount(NxJoystick* joystick)
    {
        return SDL_JoystickNumBalls(joystick);
    }

    //------------------------------------------------------
    NX_EXPORT int nxJoystickHatCount(NxJoystick* joystick)
    {
        return SDL_JoystickNumHats(joystick);
    }

    //------------------------------------------------------
    NX_EXPORT bool nxJoystickGetButton(NxJoystick* joystick, int button)
    {
        return SDL_JoystickGetButton(joystick, button - 1);
    }

    //------------------------------------------------------
    NX_EXPORT double nxJoystickGetAxis(NxJoystick* joystick, int axis)
    {
        return SDL_JoystickGetAxis(joystick, axis - 1);
    }

    //------------------------------------------------------
    NX_EXPORT bool nxJoystickGetBall(NxJoystick* joystick, int ball, int* position)
    {
        return SDL_JoystickGetBall(joystick, ball - 1, &position[0], &position[1]);
    }

    //------------------------------------------------------
    NX_EXPORT uint8_t nxJoystickGetHat(NxJoystick* joystick, int hat)
    {
        return SDL_JoystickGetHat(joystick, hat - 1);
    }

    //------------------------------------------------------
    NX_EXPORT const char* nxJoystickGetName(NxJoystick* joystick)
    {
        return SDL_JoystickName(joystick);
    }

    //------------------------------------------------------
    NX_EXPORT const char* nxJoystickGetGUID(NxJoystick* joystick)
    {
        static std::string guidStr;

        guidStr.resize(64);
        auto guid = SDL_JoystickGetGUID(joystick);
        SDL_JoystickGetGUIDString(guid, &guidStr[0], guidStr.size());

        return guidStr.data();
    }
}

//==============================================================================
