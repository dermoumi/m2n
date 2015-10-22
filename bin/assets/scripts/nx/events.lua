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
        NX_TextEntered,
        NX_TextEdited,
        NX_KeyPressed,
        NX_KeyReleased,
        NX_MouseMoved,
        NX_MouseFocus,
        NX_MouseButtonPressed,
        NX_MouseButtonReleased,
        NX_MouseWheelScrolled,
        NX_JoystickMoved,
        NX_JoystickButtonPressed,
        NX_JoystickButtonReleased,
        NX_JoystickConnect,
        NX_TouchBegan,
        NX_TouchEnded,
        NX_TouchMoved,
        NX_ClipboardUpdated,
        NX_FileDropped
    } NxEventType;

    typedef struct {
        double a, b, c;
        const char* t;
    } NxEvent;

    NxEventType nxEventWait(NxEvent*);
    NxEventType nxEventPoll(NxEvent*);
]]

local Keyboard = require 'nx.keyboard'
local Events = {}

local mouseButtons = {
    'left',
    'middle',
    'right',
    'xbutton1',
    'xbutton2'
}

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
    elseif evType == C.NX_TextEntered then
        return 'textinput', ffi.string(e.t)
    elseif evType == C.NX_TextEdited then
        return 'textedit', ffi.string(e.t), tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_KeyPressed then
        return 'keydown', Keyboard._sc[e.b], Keyboard._sym[e.c], e.a == 1
    elseif evType == C.NX_KeyReleased then
        return 'keyup', Keyboard._sc[e.b], Keyboard._sym[e.c], e.a == 1
    elseif evType == C.NX_MouseMoved then
        return 'mousemotion', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_MouseButtonPressed then
        return 'mousedown', tonumber(e.a), tonumber(e.b), mouseButtons[e.c]
    elseif evType == C.NX_MouseButtonReleased then
        return 'mouseup', tonumber(e.a), tonumber(e.b), mouseButtons[e.c]
    elseif evType == C.NX_MouseWheelScrolled then
        return 'wheelscrolled', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_JoystickMoved then
        return 'joymotion', tonumber(e.a), tonumber(e.b), tonumber(e.c)
    elseif evType == C.NX_JoystickButtonPressed then
        return 'joybuttondown', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_JoystickButtonReleased then
        return 'joybuttonup', tonumber(e.a), tonumber(e.b)
    elseif evType == C.NX_JoystickConnect then
        return 'joyconnect', tonumber(e.a), e.b == 1
    elseif evType == C.NX_TouchBegan then
        return 'touchdown', tonumber(e.a), tonumber(e.b), tonumber(e.c)
    elseif evType == C.NX_TouchEnded then
        return 'touchup', tonumber(e.a), tonumber(e.b), tonumber(e.c)
    elseif evType == C.NX_TouchMoved then
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