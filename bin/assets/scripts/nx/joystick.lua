local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    typedef struct NxJoystick NxJoystick;

    NxJoystick* nxJoystickOpen(int);
    void nxJoystickClose(NxJoystick*);
]]

--------------------------------------------------------------------------------
local Joystick = {}

local joysticks = {}

function Joystick.isConnected(id)
    -- TODO
    return false
end

function Joystick.getButtonCount(id)
    -- TODO
    return 0
end

function Joystick.getAxisCount(id)
    -- TODO
    return 0
end

function Joystick.isButtonPressed(id, button)
    -- TODO
    return false
end

function Joystick.getAxisPosition(id, axis)
    -- TODO
    return 0
end

function Joystick.getIdentification(id)
    -- TODO
    return nil, nil
end

function Joystick.__connectEvent(id, isConnected)
    if isConnected then
        joysticks[id] = ffi.gc(C.nxJoystickOpen(id), C.nxJoystickClose)
    else
        C.nxJoystickClose(ffi.gc(joysticks[id], nil))
        joysticks[id] = nil
    end
end

return Joystick