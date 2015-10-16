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
        NX_ClipboardUpdated,
        NX_FileDropped
    } NxEventType;

    typedef struct {
        double a, b, c;
        char t[32];
    } NxEvent;

    NxEventType nxEventPoll(NxEvent*);
]]

local class = require 'nx._class'
local Events = class 'Event'

-- SDL Event enum
Events.static.Quit                   = C.NX_Quit
Events.static.Resized                = C.NX_Resized
Events.static.Minimized              = C.NX_Minimized
Events.static.Maximized              = C.NX_Maximized
Events.static.Restored               = C.NX_Restored
Events.static.GainedFocus            = C.NX_GainedFocus
Events.static.LostFocus              = C.NX_LostFocus
Events.static.LowMemory              = C.NX_LowMemory
Events.static.TextEntered            = C.NX_TextEntered
Events.static.TextEdited             = C.NX_TextEdited
Events.static.KeyPressed             = C.NX_KeyPressed
Events.static.KeyReleased            = C.NX_KeyReleased
Events.static.MouseMoved             = C.NX_MouseMoved
Events.static.MouseEntered           = C.NX_MouseEntered
Events.static.MouseLeft              = C.NX_MouseLeft
Events.static.MouseButtonPressed     = C.NX_MouseButtonPressed
Events.static.MouseButtonReleased    = C.NX_MouseButtonReleased
Events.static.MouseWheelScrolled     = C.NX_MouseWheelScrolled
Events.static.JoystickMoved          = C.NX_JoystickMoved
Events.static.JoystickButtonPressed  = C.NX_JoystickButtonPressed
Events.static.JoystickButtonReleased = C.NX_JoystickButtonReleased
Events.static.JoystickConnected      = C.NX_JoystickConnected
Events.static.JoystickDisconnected   = C.NX_JoystickDisconnected
Events.static.TouchBegan             = C.NX_TouchBegan
Events.static.TouchEnded             = C.NX_TouchEnded
Events.static.TouchMoved             = C.NX_TouchMoved
Events.static.ClipboardUpdated       = C.NX_ClipboardUpdated
Events.static.FileDropped            = C.NX_FileDropped

--------------------------------------------------------------------------------
function Events.static.poll()
    local function pollFunc(t, i)
        local evType
        local evPtr = ffi.new('NxEvent[1]')
        repeat
            evType = C.nxEventPoll(evPtr)
        until evType ~= NX_Other
        event = evPtr[0]

        if evType == C.NX_NoEvent then
            return nil
        elseif evType == C.NX_Resized then
            return evType, {
                width = tonumber(event.a),
                height = tonumber(event.b)
            }
        elseif evType == C.NX_TextEntered then
            return evType, {
                text = ffi.string(event.t)
            }
        elseif evType == C.NX_TextEdited then
            return evType, {
                text = ffi.string(event.t),
                start = tonumber(event.a),
                length = tonumber(event.b)
            }
        elseif evType == C.NX_KeyPressed or evType == C.NX_KeyReleased then
            return evType, {
                -- TODO
            }
        elseif evType == C.NX_MouseMoved then
            local bit = require("bit")
            return evType, {
                x = tonumber(event.a),
                y = tonumber(event.b)
            }
        else
            return evType, nil
        end
    end
    return pollFunc, nil, nil
end

return Events