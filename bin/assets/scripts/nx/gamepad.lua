local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxGamepad NxGamepad;

    NxGamepad* nxGamepadOpen(int);
    void nxGamepadClose(NxGamepad*);    
]]

--------------------------------------------------------------------------------
local Gamepad = {}
local gamepads = {}

function Gamepad.isMapped(id)
    return gamepads[id] ~= nil
end

function Gamepad.isButtonDown(id, button)
    -- TODO
end

function Gamepad.getAxisPosition(id, axis)
    -- TODO
end

function Gamepad.loadMappings(mappings)
    -- TODO
end

function Gamepad.saveMappings(filename)
    -- TODO
end

function Gamepad.setMapping(guid, target, inputType, inputIndex, hatDir)
    -- TODO
end

function Gamepad.__connectEvent(id, isConnected)
    if isConnected then
        gamepads[id] = ffi.gc(C.nxGamepadOpen(id), C.nxGamepadClose)
    elseif gamepads[id] ~= nil then
        C.nxGamepadClose(ffi.gc(gamepads[id], nil))
        gamepads[id] = nil
    end
end

return Gamepad