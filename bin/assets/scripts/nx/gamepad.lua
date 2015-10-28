local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxGamepad NxGamepad;

    NxGamepad* nxGamepadOpen(int);
    void nxGamepadClose(NxGamepad*);
    bool nxGamepadButtonDown(NxGamepad*, int);
    double nxGamepadGetAxis(NxGamepad*, int);
    bool nxGamepadAddMapping(const char*);
    const char* nxGamepadGetMapping(const char*);
    bool nxGamepadAddMappings(const char*);
    const char* nxGamepadGetMappings();
]]

--------------------------------------------------------------------------------
local Gamepad = {}
local gamepads = {}
local InputFile = require 'nx.inputfile'
local OutputFile = require 'nx.outputfile'

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

local mappingTarget = {
    a = 'a',
    b = 'b',
    x = 'x',
    y = 'y',
    back = 'back',
    guide = 'guide',
    start = 'start',
    left = 'dpleft',
    right = 'dpright',
    up = 'dpup',
    down = 'dpdown',
    l = 'leftshoulder',
    r = 'rightshoulder',
    lstick = 'leftstick',
    rstick = 'rightstick',
    ltrigger = 'lefttrigger',
    rtrigger = 'righttrigger',
    leftx = 'leftx',
    lefty = 'lefty',
    rightx = 'rightx',
    righty = 'righty'
}

local mappingType = {
    button = 'b',
    hat = 'h',
    axis = 'a'
}

local mappingHat = {
    centered  = 0,
    up        = 1,
    right     = 2,
    down      = 4,
    left      = 8,
    rightup   = 3,
    rightdown = 6,
    leftup    = 9,
    leftdown  = 12
}

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
    local mappingsStr = ""
    local file, err = InputFile:new(mappings)
    if file then
        mappingsStr = file:read(file:size())
        file:close()
    else
        mappingsStr = mappings
    end

    return C.nxGamepadAddMappings(mappingsStr)
end

function Gamepad.saveMappings(filename)
    local mappings = ffi.string(C.nxGamepadGetMappings())

    if filename then
        local file, err = OutputFile:new(filename)
        if file then
            file:write(mappings)
            file:close()
        end
    end

    return mappings
end

function Gamepad.setMapping(guid, mappings)
    -- Get the GUID if we're given the joystick ID
    if type(guid) == 'number' then
        local Joystick = require 'nx.joystick'
        guid = Joystick.getGUID(id)
        if guid == '' then return false, 'Invalid Joystick ID' end
    end

    -- Build up the mappings string if 'mappings' is an array
    local mappingsStr = ''
    if type(mappings) == 'string' then
        mappingsStr = guid .. ',Controller,' .. mappings
    elseif type(mappings) == 'table' then
        mappingsStr = guid .. ',Controller,'
        for target, data in pairs(mappings) do
            target = mappingTarget[target]
            if target then
                -- Make sure all needed data is valid
                if type(data) == 'table' and mappingType[data.type] and
                    type(data.index) == 'number' and
                    (data.type ~= 'hat' or mappingHat[data.hat])
                then
                    local dataStr = mappingType[data.type] .. data.index
                    if data.type == 'hat' then
                        dataStr = dataStr .. '.' .. mappingHat[data.hat]
                    end

                    -- Override data
                    data = dataStr
                end

                -- Make sure the data to add is an actual string and not some invalid table
                if type(data) == 'string' then
                    mappingsStr = mappingsStr .. target .. ':' .. data .. ','
                end
            end
        end
    end

    return C.nxGamepadAddMapping(mappingsStr)
end

function Gamepad.getMapping(guid)
    -- Get the GUID if we're given the joystick ID
    if type(guid) == 'number' then
        local Joystick = require 'nx.joystick'
        guid = Joystick.getGUID(id)
        if guid == '' then return nil, 'Invalid Joystick ID' end
    end

    local mapping = C.nxGamepadGetMapping(guid)
    if mapping == nil then return nil end
    mapping = ffi.string(mapping)

    -- TODO: Parse it into data
    return mapping
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