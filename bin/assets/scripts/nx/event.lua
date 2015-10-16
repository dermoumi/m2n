local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef enum {
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
        NX_FileDropped,
        NX_AudioDeviceAdded,
        NX_AudioDeviceRemoved
    } EventType;

    uint32_t nxEventPoll();
]]

local class = require 'nx._class'
local Event = class 'Event'

-- SDL Event enum
Event.static.Quit                   = C.NX_Quit
Event.static.Resized                = C.NX_Resized
Event.static.Minimized              = C.NX_Minimized
Event.static.Maximized              = C.NX_Maximized
Event.static.Restored               = C.NX_Restored
Event.static.GainedFocus            = C.NX_GainedFocus
Event.static.LostFocus              = C.NX_LostFocus
Event.static.LowMemory              = C.NX_LowMemory
Event.static.TextEntered            = C.NX_TextEntered
Event.static.TextEdited             = C.NX_TextEdited
Event.static.KeyPressed             = C.NX_KeyPressed
Event.static.KeyReleased            = C.NX_KeyReleased
Event.static.MouseMoved             = C.NX_MouseMoved
Event.static.MouseEntered           = C.NX_MouseEntered
Event.static.MouseLeft              = C.NX_MouseLeft
Event.static.MouseButtonPressed     = C.NX_MouseButtonPressed
Event.static.MouseButtonReleased    = C.NX_MouseButtonReleased
Event.static.MouseWheelScrolled     = C.NX_MouseWheelScrolled
Event.static.JoystickMoved          = C.NX_JoystickMoved
Event.static.JoystickButtonPressed  = C.NX_JoystickButtonPressed
Event.static.JoystickButtonReleased = C.NX_JoystickButtonReleased
Event.static.JoystickConnected      = C.NX_JoystickConnected
Event.static.JoystickDisconnected   = C.NX_JoystickDisconnected
Event.static.TouchBegan             = C.NX_TouchBegan
Event.static.TouchEnded             = C.NX_TouchEnded
Event.static.TouchMoved             = C.NX_TouchMoved
Event.static.SensorChanged          = C.NX_SensorChanged
Event.static.ClipboardUpdated       = C.NX_ClipboardUpdated
Event.static.FileDropped            = C.NX_FileDropped
Event.static.AudioDeviceAdded       = C.NX_AudioDeviceAdded
Event.static.AudioDeviceRemoved     = C.NX_AudioDeviceRemoved

--[[
local SizeEvent = class 'SizeEvent'

function SizeEvent:initialize(width, height)
    self.width = width
    self.height = height
end
]]

function Event.static.poll()
    local function pollFunc(t, i)
        local evType
        repeat
            evType = C.nxEventPoll()
        until evType ~= NX_Other

        if evType ~= C.NX_NoEvent then
            return evType, {}
        end
    end
    return pollFunc, nil, nil
end

return Event