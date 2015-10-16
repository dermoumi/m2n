#include "../config.hpp"
#include <SDL2/SDL.h>
#include <string>

using NxWindow = SDL_Window;
extern NxWindow* nxWindowGet();

enum NxEventType {
    NX_NoEvent = 0,
    NX_Other,
    NX_Quit,
    NX_Resized,
    NX_Minimized,
    NX_Maximized,
    NX_Restored,
    NX_GainedFocus,
    NX_LostFocus,
    NX_LowMemory,
    NX_TextEntered,
    NX_TextEdited,
    NX_KeyPressed,
    NX_KeyReleased,
    NX_MouseMoved,
    NX_MouseEntered,
    NX_MouseLeft,
    NX_MouseButtonPressed,
    NX_MouseButtonReleased,
    NX_MouseWheelScrolled,
    NX_JoystickMoved,
    NX_JoystickButtonPressed,
    NX_JoystickButtonReleased,
    NX_JoystickConnected,
    NX_JoystickDisconnected,
    NX_TouchBegan,
    NX_TouchEnded,
    NX_TouchMoved,
    NX_ClipboardUpdated,
    NX_FileDropped
};

struct NxEvent {
    double a, b, c, d, e;
    const char* t;
};

namespace
{
    double toNxMouseButtons(uint8_t button)
    {
        switch (button) {
        case SDL_BUTTON_LEFT:
            return 0.0;
        case SDL_BUTTON_RIGHT:
            return 1.0;
        case SDL_BUTTON_MIDDLE:
            return 2.0;
        case SDL_BUTTON_X1:
            return 3.0;
        case SDL_BUTTON_X2:
        default:
            return 4.0;
        }
    }
}
extern "C"
{
    NX_EXPORT NxEventType nxEventPoll(NxEvent* e)
    {
        static std::string strArg;

        SDL_Event event;
        int pending = SDL_PollEvent(&event);

        if (pending == 0) return NX_NoEvent;

        if (event.type == SDL_WINDOWEVENT) {
        }

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
                        return NX_Minimized;
                    case SDL_WINDOWEVENT_MAXIMIZED:
                        return NX_Maximized;
                    case SDL_WINDOWEVENT_RESTORED:
                        return NX_Restored;
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        return NX_GainedFocus;
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                        return NX_LostFocus;
                    case SDL_WINDOWEVENT_ENTER:
                        return NX_MouseEntered;
                    case SDL_WINDOWEVENT_LEAVE:
                        return NX_MouseLeft;
                    default:
                        return NX_Other;
                }
            case SDL_APP_LOWMEMORY:
                return NX_LowMemory;
            case SDL_TEXTINPUT:
                strArg = event.text.text;
                e->t = strArg.data();
                return NX_TextEntered;
            case SDL_TEXTEDITING:
                strArg = event.text.text;
                e->a = event.edit.start;
                e->b = event.edit.length;
                e->t = strArg.data();
                return NX_TextEdited;
            case SDL_KEYDOWN:
                // TODO
                return NX_KeyPressed;
            case SDL_KEYUP:
                // TODO
                return NX_KeyReleased;
            case SDL_MOUSEMOTION:
                if (event.motion.which == SDL_TOUCH_MOUSEID) return NX_Other;
                e->a = event.motion.x;
                e->b = event.motion.y;
                return NX_MouseMoved;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.which == SDL_TOUCH_MOUSEID) return NX_Other;
                e->a = event.button.x;
                e->b = event.button.y;
                e->c = toNxMouseButtons(event.button.button);
                return NX_MouseButtonPressed;
            case SDL_MOUSEBUTTONUP:
                if (event.button.which == SDL_TOUCH_MOUSEID) return NX_Other;
                e->a = event.button.x;
                e->b = event.button.y;
                e->c = toNxMouseButtons(event.button.button);
                return NX_MouseButtonReleased;
            case SDL_MOUSEWHEEL:
                e->a = event.wheel.x;
                e->b = event.wheel.y;
                return NX_MouseWheelScrolled;
            case SDL_CONTROLLERAXISMOTION:
                e->a = event.caxis.which;
                e->b = event.caxis.axis;
                e->c = event.caxis.value;
                return NX_JoystickMoved;
            case SDL_CONTROLLERBUTTONDOWN:
                e->a = event.cbutton.which;
                e->b = event.cbutton.button;
                return NX_JoystickButtonPressed;
            case SDL_CONTROLLERBUTTONUP:
                e->a = event.cbutton.which;
                e->b = event.cbutton.button;
                return NX_JoystickButtonReleased;
            case SDL_CONTROLLERDEVICEADDED:
                e->a = event.cdevice.which;
                return NX_JoystickConnected;
            case SDL_CONTROLLERDEVICEREMOVED:
                e->a = event.cdevice.which;
                return NX_JoystickDisconnected;
            case SDL_FINGERDOWN:
                e->a = event.tfinger.fingerId;
                e->b = event.tfinger.touchId;
                e->c = event.tfinger.pressure;
                e->d = event.tfinger.x;
                e->e = event.tfinger.y;
                return NX_TouchBegan;
            case SDL_FINGERUP:
                e->a = event.tfinger.fingerId;
                e->b = event.tfinger.touchId;
                e->c = event.tfinger.pressure;
                e->d = event.tfinger.x;
                e->e = event.tfinger.y;
                return NX_TouchEnded;
            case SDL_FINGERMOTION:
                e->a = event.tfinger.fingerId;
                e->b = event.tfinger.touchId;
                e->c = event.tfinger.pressure;
                e->d = event.tfinger.x;
                e->e = event.tfinger.y;
                return NX_TouchMoved;
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
}