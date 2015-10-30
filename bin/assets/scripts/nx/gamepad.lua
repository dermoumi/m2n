--[[----------------------------------------------------------------------------
    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org>
--]]----------------------------------------------------------------------------

------------------------------------------------------------
-- ffi C declarations
------------------------------------------------------------
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

------------------------------------------------------------
-- A set of functions to interact with Controller devices
------------------------------------------------------------
local Gamepad = {}

local InputFile = require 'nx.inputfile'
local OutputFile = require 'nx.outputfile'

-- Constants -----------------------------------------------
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

local mappingFromTarget = {}
for i, v in pairs(mappingTarget) do
    mappingFromTarget[v] = i
end

local mappingType = {
    button = 'b',
    hat = 'h',
    axis = 'a'
}
local mappingFromType = {}
for i, v in pairs(mappingType) do
    mappingFromType[v] = i
end

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

-- Local variables -----------------------------------------
local gamepads = {}

------------------------------------------------------------
function Gamepad.isMapped(id)
    return gamepads[id] ~= nil
end

------------------------------------------------------------
function Gamepad.isButtonDown(id, button)
    if not gamepads[id] or not buttons[button] then return false end

    return C.nxGamepadButtonDown(gamepads[id], buttons[button])
end

------------------------------------------------------------
function Gamepad.getAxisPosition(id, axis)
    if not gamepads[id] or not axes[axis] then return 0 end
    return C.nxGamepadGetAxis(gamepads[id], axes[axis])
end

------------------------------------------------------------
function Gamepad.loadMappings(mappings)
    -- Make sure the arguments are valid
    if type(mappings) == 'string' or mappings == '' then
        return false, 'Invalid arguments'
    end

    -- Attempt to load from file if said file exists
    local mappingsStr = ''
    local file, err = InputFile:new(mappings)
    if file then
        mappingsStr = file:read(file:size())
        file:close()
    else
        mappingsStr = mappings
    end

    return C.nxGamepadAddMappings(mappingsStr)
end

------------------------------------------------------------
function Gamepad.saveMappings(filename)
    -- Retrieve mappings string
    local mappings = ffi.string(C.nxGamepadGetMappings())

    -- Attempt to save to file if filename is valid
    if type(filename) == 'string' and filename ~= '' then
        local file, err = OutputFile:new(filename)
        if file then
            file:write(mappings)
            file:close()
        end
    end

    -- Return retrieved mappings
    return mappings
end

------------------------------------------------------------
function Gamepad.setMapping(guid, mappings)
    -- Get the GUID if we're given the joystick ID
    if type(guid) == 'number' then
        local Joystick = require 'nx.joystick'
        guid = Joystick.getGUID(id)
    end

    -- Check arguments
    if type(guid) ~= 'string' or guid == '' then
        return false, 'Invalid arguments'
    end

    -- Build up the mappings string if 'mappings' is an array
    local mappingsStr = ''
    if type(mappings) == 'string' then
        -- Append the mappings to the GUID and a generic controller name
        mappingsStr = guid .. ',Controller,' .. mappings
    elseif type(mappings) == 'table' then
        -- Convert mapping- to strings and append them to the GUID and a generic name
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
    else
        -- Invalid argument
        return false, 'Invalid arguments'
    end

    return C.nxGamepadAddMapping(mappingsStr)
end

------------------------------------------------------------
function Gamepad.getMapping(guid, raw)
    -- Get the GUID if we're given the joystick ID
    if type(guid) == 'number' then
        local Joystick = require 'nx.joystick'
        guid = Joystick.getGUID(id)
        if guid == '' then return {}, 'Invalid Joystick ID' end
    end

    local mapping = C.nxGamepadGetMapping(guid)
    if mapping == nil then return {}, 'No mapping available' end
    mapping = ffi.string(mapping)

    -- Return it as is if requested raw
    if raw then return mapping end

    -- Parse it into a lua table
    local data = {}
    local dataParsed = 0
    local target, type, index, hat

    -- Split by comma :/
    for i in mapping:gmatch('[^,]+') do
        if dataParsed < 2 then
            dataParsed = dataParsed + 1
        else
            _target, _type, _index = i:match('^(.+):([ba])(%d+)$')
            if _target then
                data[mappingFromTarget[_target]] = {
                    type = mappingFromType[_type],
                    index = _index
                }
            else
                _target, _type, _index, _hat = i:match('^(.+):(h)(%d+)%.(%d+)$')
                if _target then
                    data[mappingFromTarget[_target]] = {
                        type = 'hat',
                        index = _index,
                        hat = _hat
                    }
                end
            end
        end
    end

    return data
end

-- Automatically called on gamepadconnect events -----------
function Gamepad.__connectEvent(id, isConnected)
    if isConnected then
        gamepads[id] = ffi.gc(C.nxGamepadOpen(id), C.nxGamepadClose)
    elseif gamepads[id] ~= nil then
        C.nxGamepadClose(ffi.gc(gamepads[id], nil))
        gamepads[id] = nil
    end
end

------------------------------------------------------------
return Gamepad