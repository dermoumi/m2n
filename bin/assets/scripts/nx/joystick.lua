local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    // TODO
]]

--------------------------------------------------------------------------------
local Joystick = {}

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

return Joystick