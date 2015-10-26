local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxGamepad NxGamepad;

    NxGamepad* nxGamepadOpen(int);
    void nxGamepadClose(NxGamepad*);
    bool nxGamepadButtonDown(NxGamepad*, int);
    double nxGamepadGetAxis(NxGamepad*, int);
]]

--------------------------------------------------------------------------------
local Gamepad = {}
local gamepads = {}

local axes = {
    [0] = 'invalid',
    'leftx',
    'lefty',
    'rightx',
    'righty',
    'ltrigger',
    'rtrigger',
    invalid = 0,
    leftx = 1,
    lefty = 2,
    rightx = 3,
    righty = 4,
    ltrigger = 5,
    rtrigger = 6
}
Gamepad._axes = axes

local buttons = {
    [0] = 'invalid',
    'a',
    'b',
    'x',
    'y',
    'back',
    'guide',
    'start',
    'lstick',
    'rstick',
    'l',
    'r',
    'up',
    'down',
    'left',
    'right',
    invalid = 0,
    a = 1,
    b = 2,
    x = 3,
    y = 4,
    back = 5,
    guide = 6,
    start = 7,
    lstick = 8,
    rstick = 9,
    l = 10,
    r = 11,
    up = 12,
    down = 13,
    left = 14,
    right = 15
}
Gamepad._buttons = buttons

function Gamepad.isMapped(id)
    return gamepads[id] ~= nil
end

function Gamepad.isButtonDown(id, button)
    if not gamepads[id] then return false end
    return C.nxGamepadButtonDown(gamepads[id], buttons[button])
end

function Gamepad.getAxisPosition(id, axis)
    if not gamepads[id] then return 0 end
    return C.nxGamepadGetAxis(gamepads[id], axes[axis])
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