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
// Constants
//----------------------------------------------------------
enum NxEventType
{
    NX_NoEvent = 0,
    NX_Other,
    NX_Quit,
    NX_Resized,
    NX_Visible,
    NX_Focus,
    NX_LowMemory,
    NX_TextInput,
    NX_TextEdit,
    NX_KeyDown,
    NX_KeyUp,
    NX_MouseMotion,
    NX_MouseFocus,
    NX_MouseDown,
    NX_MouseUp,
    NX_WheelScroll,
    NX_JoyAxisMotion,
    NX_JoyBallMotion,
    NX_JoyHatMotion,
    NX_JoyButtonDown,
    NX_JoyButtonUp,
    NX_JoyConnect,
    NX_GamepadMotion,
    NX_GamepadButtonDown,
    NX_GamepadButtonUp,
    NX_GamepadConnect,
    NX_GamepadRemap,
    NX_TouchBegan,
    NX_TouchEnded,
    NX_TouchMotion,
    NX_ClipboardUpdated,
    NX_FileDropped
};

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxWindow = SDL_Window;
extern NxWindow* nxWindowGet();

struct NxEvent {
    double a, b, c, d;
    const char* t;
};

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
static NxEventType nextEvent(NxEvent* e, int (*func)(SDL_Event*))
{
    static std::string strArg;

    SDL_Event event;
    int pending = func(&event);

    if (pending == 0) return NX_NoEvent;

    switch (event.type) {
    case SDL_QUIT:
        return NX_Quit;
    case SDL_WINDOWEVENT:
        switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                e->a = event.window.data1;
                e->b = event.window.data2;
                return NX_Resized;
            case SDL_WINDOWEVENT_MINIMIZED:
                e->a = 0.0;
                return NX_Visible;
            case SDL_WINDOWEVENT_RESTORED:
                e->a = 1.0;
                return NX_Visible;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                e->a = 1.0;
                return NX_Focus;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                e->a = 0.0;
                return NX_Focus;
            case SDL_WINDOWEVENT_ENTER:
                e->a = 1.0;
                return NX_MouseFocus;
            case SDL_WINDOWEVENT_LEAVE:
                e->b = 0.0;
                return NX_MouseFocus;
            default:
                return NX_Other;
        }
    case SDL_APP_LOWMEMORY:
        return NX_LowMemory;
    case SDL_TEXTINPUT:
        strArg = event.text.text;
        e->t = strArg.data();
        return NX_TextInput;
    case SDL_TEXTEDITING:
        strArg = event.text.text;
        e->a = event.edit.start;
        e->b = event.edit.length;
        e->t = strArg.data();
        return NX_TextEdit;
    case SDL_KEYDOWN:
        e->a = event.key.keysym.scancode;
        e->b = event.key.keysym.sym;
        e->c = event.key.repeat;
        return NX_KeyDown;
    case SDL_KEYUP:
        e->a = event.key.keysym.scancode;
        e->b = event.key.keysym.sym;
        return NX_KeyUp;
    case SDL_MOUSEMOTION:
        if (event.motion.which == SDL_TOUCH_MOUSEID) return NX_Other;
        e->a = event.motion.x;
        e->b = event.motion.y;
        e->c = event.motion.xrel;
        e->d = event.motion.yrel;
        return NX_MouseMotion;
    case SDL_MOUSEBUTTONDOWN:
        if (event.button.which == SDL_TOUCH_MOUSEID) return NX_Other;
        e->a = event.button.x;
        e->b = event.button.y;
        e->c = event.button.button;
        return NX_MouseDown;
    case SDL_MOUSEBUTTONUP:
        if (event.button.which == SDL_TOUCH_MOUSEID) return NX_Other;
        e->a = event.button.x;
        e->b = event.button.y;
        e->c = event.button.button;
        return NX_MouseUp;
    case SDL_MOUSEWHEEL:
        e->a = event.wheel.x;
        e->b = event.wheel.y;
        return NX_WheelScroll;
    case SDL_JOYAXISMOTION:
        e->a = event.jaxis.which + 1;
        e->b = event.jaxis.axis + 1;
        e->c = event.jaxis.value;
        return NX_JoyAxisMotion;
    case SDL_JOYBALLMOTION:
        e->a = event.jball.which + 1;
        e->b = event.jball.ball + 1;
        e->c = event.jball.xrel;
        e->d = event.jball.yrel;
        return NX_JoyBallMotion;
    case SDL_JOYHATMOTION:
        e->a = event.jhat.which + 1;
        e->b = event.jhat.hat + 1;
        e->c = event.jhat.value;
        return NX_JoyHatMotion;
    case SDL_JOYBUTTONDOWN:
        e->a = event.jbutton.which + 1;
        e->b = event.jbutton.button + 1;
        return NX_JoyButtonDown;
    case SDL_JOYBUTTONUP:
        e->a = event.jbutton.which + 1;
        e->b = event.jbutton.button + 1;
        return NX_JoyButtonUp;
    case SDL_JOYDEVICEADDED:
        e->a = event.jdevice.which + 1;
        e->b = 1.0;
        return NX_JoyConnect;
    case SDL_JOYDEVICEREMOVED:
        e->a = event.jdevice.which + 1;
        e->b = 0.0;
        return NX_JoyConnect;
    case SDL_CONTROLLERAXISMOTION:
        e->a = event.caxis.which + 1;
        e->b = event.caxis.axis + 1;
        e->c = event.caxis.value;
        return NX_GamepadMotion;
    case SDL_CONTROLLERBUTTONDOWN:
        e->a = event.cbutton.which + 1;
        e->b = event.cbutton.button + 1;
        return NX_GamepadButtonDown;
    case SDL_CONTROLLERBUTTONUP:
        e->a = event.cbutton.which + 1;
        e->b = event.cbutton.button + 1;
        return NX_GamepadButtonUp;
    case SDL_CONTROLLERDEVICEADDED:
        e->a = event.cdevice.which + 1;
        e->b = 1.0;
        return NX_GamepadConnect;
    case SDL_CONTROLLERDEVICEREMOVED:
        e->a = event.cdevice.which + 1;
        e->b = 0.0;
        return NX_GamepadConnect;
    case SDL_CONTROLLERDEVICEREMAPPED:
        e->a = event.cdevice.which + 1;
        return NX_GamepadRemap;
    case SDL_FINGERDOWN:
        e->a = event.tfinger.fingerId;
        e->b = event.tfinger.x;
        e->c = event.tfinger.y;
        return NX_TouchBegan;
    case SDL_FINGERUP:
        e->a = event.tfinger.fingerId;
        e->b = event.tfinger.x;
        e->c = event.tfinger.y;
        return NX_TouchEnded;
    case SDL_FINGERMOTION:
        e->a = event.tfinger.fingerId;
        e->b = event.tfinger.x;
        e->c = event.tfinger.y;
        return NX_TouchMotion;
    case SDL_CLIPBOARDUPDATE:
        return NX_ClipboardUpdated;
    case SDL_DROPFILE:
        strArg = event.drop.file;
        SDL_free(event.drop.file);
        e->t = strArg.data();
        return NX_FileDropped;
    default:
        return NX_Other;
    }
}

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT NxEventType nxEventWait(NxEvent* e)
{
    return nextEvent(e, SDL_WaitEvent);
}

//----------------------------------------------------------
NX_EXPORT NxEventType nxEventPoll(NxEvent* e)
{
    return nextEvent(e, SDL_PollEvent);
}

//==============================================================================
