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
        const char* t;
    } NxEvent;

    NxEventType nxEventPoll(NxEvent*);
]]

local Events = {}

local mouseButtons = {
    'left',
    'middle',
    'right',
    'xbutton1',
    'xbutton2'
}

--------------------------------------------------------------------------------
function Events.poll()
    local function pollFunc(t, i)
        local evType
        local evPtr = ffi.new('NxEvent[1]')
        repeat
            evType = C.nxEventPoll(evPtr)
        until evType ~= NX_Other
        e = evPtr[0]

        if evType == C.NX_NoEvent then
            return nil
        elseif evType == C.NX_Quit then
            return 'quit'
        elseif evType == C.NX_Resized then
            return 'resized', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_Visible then
            return 'visible', (e.a == 1.0)
        elseif evType == C.NX_Focus then
            return 'focus', (e.a == 1.0)
        elseif evType == C.NX_MouseFocus then
            return 'mousefocus', (e.a == 1.0)
        elseif evType == C.NX_TextEntered then
            return 'textentered', ffi.string(e.t)
        elseif evType == C.NX_TextEdited then
            return 'textedited', ffi.string(e.t), tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_KeyPressed then
            return 'keypressed', nil -- TODO
        elseif evType == C.NX_KeyReleased then
            return 'keyreleased', nil -- TODO
        elseif evType == C.NX_MouseMoved then
            return 'mousemoved', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_MouseButtonPressed then
            return 'mousepressed', tonumber(e.a), tonumber(e.b), mouseButtons[e.c]
        elseif evType == C.NX_MouseButtonReleased then
            return 'mousereleased', tonumber(e.a), tonumber(e.b), mouseButtons[e.c]
        elseif evType == C.NX_MouseWheelScrolled then
            return 'mousewheelscrolled', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_JoystickMoved then
            return 'joymoved', tonumber(e.a), tonumber(e.b), tonumber(e.c)
        elseif evType == C.NX_JoystickButtonPressed then
            return 'joybuttonpressed', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_JoystickButtonReleased then
            return 'joybuttonreleasd', tonumber(e.a), tonumber(e.b)
        elseif evType == C.NX_JoystickConnected then
            return 'joyconnected', tonumber(e.a)
        elseif evType == C.NX_JoystickDisconnected then
            return 'joydisconnected', tonumber(e.a)
        elseif evType == C.NX_TouchBegan then
            return 'touchbegan', tonumber(e.a), tonumber(e.b), tonumber(e.c)
        elseif evType == C.NX_TouchEnded then
            return 'touchended', tonumber(e.a), tonumber(e.b), tonumber(e.c)
        elseif evType == C.NX_TouchMoved then
            return 'touchmoved', tonumber(e.a), tonumber(e.b), tonumber(e.c)
        elseif evType == C.NX_FileDropped then
            return 'filedropped', ffi.string(e.t)
        else
            return 'other'
        end
    end
    return pollFunc, nil, nil
end

return Events