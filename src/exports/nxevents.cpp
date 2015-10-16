#include "../config.hpp"
#include <SDL2/SDL.h>

using NxWindow = SDL_Window;
extern NxWindow* nxWindowGet();

enum EventType {
    NX_NoEvent = 0,
    NX_Quit,
    NX_Resized,
    NX_Minimized,
    NX_Maximized,
    NX_Restored,
    NX_LostFocus,
    NX_GainedFocus,
    NX_LowMemory,
    NX_WillEnterBg,
    NX_DidEnterBg,
    NX_WillEnterFg,
    NX_DidEnterFg,
    NX_TextEntered,
    NX_KeyPressed,
    NX_KeyReleased,
    NX_MouseMoved,
    NX_MouseEntered,
    NX_MouseLeft,
    NX_MouseButtonPressed,
    NX_MouseButtonReleased,
    NX_MouseWheelScrolled,
    NX_JoystickMoved,
    NX_JoystickConnected,
    NX_JoystickDisconnected,
    NX_TouchBegan,
    NX_TouchEnded,
    NX_TouchMoved,
    NX_SensorChanged,
    NX_ClipboardUpdated,
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

        switch (event.type) {
            case SDL_QUIT:      return NX_Quit;
            default:            return NX_NoEvent;
        }
    }
}