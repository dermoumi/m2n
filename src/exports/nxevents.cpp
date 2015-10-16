#include "../config.hpp"
#include <SDL2/SDL.h>

using NxWindow = SDL_Window;
extern NxWindow* nxWindowGet();

enum EventType {
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
    NX_SensorChanged,
    NX_ClipboardUpdated,
    Nx_FileDropped,
    NX_AudioDeviceAdded,
    NX_AudioDeviceRemoved
};

extern "C"
{
    NX_EXPORT uint32_t nxEventPoll()
    {
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
                return NX_TextEntered;
            case SDL_TEXTEDITING:
                return NX_TextEdited;
            case SDL_KEYDOWN:
                return NX_KeyPressed;
            case SDL_KEYUP:
                return NX_KeyReleased;
            case SDL_MOUSEMOTION:
                return NX_MouseMoved;
            case SDL_MOUSEBUTTONDOWN:
                return NX_MouseButtonPressed;
            case SDL_MOUSEBUTTONUP:
                return NX_MouseButtonReleased;
            case SDL_MOUSEWHEEL:
                return NX_MouseWheelScrolled;
            case SDL_CONTROLLERAXISMOTION:
                return NX_JoystickMoved;
            case SDL_CONTROLLERBUTTONDOWN:
                return NX_JoystickButtonPressed;
            case SDL_CONTROLLERBUTTONUP:
                return NX_JoystickButtonReleased;
            case SDL_CONTROLLERDEVICEADDED:
                return NX_JoystickConnected;
            case SDL_CONTROLLERDEVICEREMOVED:
                return NX_JoystickDisconnected;
            case SDL_FINGERDOWN:
                return NX_TouchBegan;
            case SDL_FINGERUP:
                return NX_TouchEnded;
            case SDL_FINGERMOTION:
                return NX_TouchMoved;
            default:
                return NX_Other;
        }
    }
}