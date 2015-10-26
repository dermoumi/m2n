local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef enum {
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
        NX_WheelScrolled,
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
    } NxEventType;

    typedef struct {
        double a, b, c, d;
        const char* t;
    } NxEvent;

    NxEventType nxEventWait(NxEvent*);
    NxEventType nxEventPoll(NxEvent*);
]]

local Keyboard = require 'nx.keyboard'
local Mouse = require 'nx.mouse'
local Joystick = require 'nx.joystick'
local Gamepad = require 'nx.gamepad'
local Events = {}

--------------------------------------------------------------------------------
local function nextEvent(func)
    local evType
    local evPtr = ffi.new('NxEvent[1]')
    repeat
        evType = func(evPtr)
    until evType ~= NX_Other
    e = evPtr[0]

    if evType == C.NX_NoEvent then
        return nil
    elseif evType == C.NX_Quit then
        return 'quit'
    elseif evType == C.NX_Resized then
        return 'resized', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_Visible then
        return 'visible', e.a == 1
    elseif evType == C.NX_Focus then
        return 'focus', e.a == 1
    elseif evType == C.NX_MouseFocus then
        return 'mousefocus', (e.a == 1.0)
    elseif evType == C.NX_TextInput then
        return 'textinput', ffi.string(e.t)
    elseif evType == C.NX_TextEdit then
        return 'textedit', ffi.string(e.t), tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_KeyDown then
        return 'keydown', Keyboard._sc[e.b], Keyboard._sym[e.c], e.a == 1
    elseif evType == C.NX_KeyUp then
        return 'keyup', Keyboard._sc[e.b], Keyboard._sym[e.c], e.a == 1
    elseif evType == C.NX_MouseMotion then
        return 'mousemotion', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_MouseDown then
        return 'mousedown', tonumber(e.a), tonumber(e.b), Mouse._btn[e.c]
    elseif evType == C.NX_MouseUp then
        return 'mouseup', tonumber(e.a), tonumber(e.b), Mouse._btn[e.c]
    elseif evType == C.NX_WheelScrolled then
        return 'wheelscroll', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_JoyAxisMotion then
        return 'joyaxismotion', tonumber(e.a), tonumber(e.b), tonumber(e.c)
    elseif evType == C.NX_JoyBallMotion then
        return 'joyhatmotion', tonumber(e.a), tonumber(e.b), tonumber(e.c), tonumber(e.d)
    elseif evType == C.NX_JoyHatMotion then
        return 'joyballmotion', tonumber(e.a), tonumber(e.b), tonumber(e.c)
    elseif evType == C.NX_JoyButtonDown then
        return 'joybuttondown', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_JoyButtonUp then
        return 'joybuttonup', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_JoyConnect then
        Joystick.__connectEvent(tonumber(e.a), e.b == 1)
        return 'joyconnect', tonumber(e.a), e.b == 1
    elseif evType == C.NX_GamepadMotion then
        return 'gamepadmotion', tonumber(e.a), tonumber(e.b), tonumber(e.c) -- TODO: Add axis name
    elseif evType == C.NX_GamepadButtonDown then
        return 'gamepadbuttondown', tonumber(e.a), tonumber(e.b), tonumber(e.c) -- TODO: Add button name
    elseif evType == C.NX_GamepadButtonUp then
        return 'gamepadbuttonup', tonumber(e.a), tonumber(e.b), tonumber(e.c) -- TODO: Add button name
    elseif evType == C.NX_GamepadConnect then
        Gamepad.__connectEvent(tonumber(e.a), e.b == 1)
        return 'gamepadconnect', tonumber(e.a), e.b == 1
    elseif evType == C.NX_GamepadRemap then
        return 'gamepadremap', tonumber(e.a)
    elseif evType == C.NX_TouchBegan then
        return 'touchdown', tonumber(e.a), tonumber(e.b), tonumber(e.c)
    elseif evType == C.NX_TouchEnded then
        return 'touchup', tonumber(e.a), tonumber(e.b), tonumber(e.c)
    elseif evType == C.NX_TouchMotion then
        return 'touchmotion', tonumber(e.a), tonumber(e.b), tonumber(e.c)
    elseif evType == C.NX_FileDropped then
        return 'filedropped', ffi.string(e.t)
    else
        return 'other'
    end
end

--------------------------------------------------------------------------------
function Events.wait()
    return function(t, i)
        return nextEvent(C.nxEventWait)
    end
end

function Events.poll()
    return function(t, i)
        return nextEvent(C.nxEventPoll)
    end
end

return Events