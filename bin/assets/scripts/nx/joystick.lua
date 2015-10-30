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

ffi.cdef[[
    typedef struct NxJoystick NxJoystick;

    NxJoystick* nxJoystickOpen(int);
    void nxJoystickClose(NxJoystick*);
    int nxJoystickButtonCount(NxJoystick*);
    int nxJoystickAxisCount(NxJoystick*);
    int nxJoystickBallCount(NxJoystick*);
    int nxJoystickHatCount(NxJoystick*);
    bool nxJoystickGetButton(NxJoystick*, int);
    double nxJoystickGetAxis(NxJoystick*, int);
    bool nxJoystickGetBall(NxJoystick*, int, int*);
    uint8_t nxJoystickGetHat(NxJoystick*, int);
    const char* nxJoystickGetName(NxJoystick*);
    const char* nxJoystickGetGUID(NxJoystick*);
]]

------------------------------------------------------------
-- A set of functions to interact with Joystick devices
------------------------------------------------------------
local Joystick = {}

-- Constants -----------------------------------------------
local hatPos = {
    [0] = 'centered',
    [1] = 'up',
    [2] = 'right',
    [4] = 'down',
    [8] = 'left',
    [3] = 'rightup',
    [6] = 'rightdown',
    [9] = 'leftup',
    [12] = 'leftdown'
}
Joystick._hatPos = hatPos

-- Local variables -----------------------------------------
local joysticks = {}

------------------------------------------------------------
function Joystick.isConnected(id)
    return joysticks[id] ~= nil
end

------------------------------------------------------------
function Joystick.getButtonCount(id)
    if not joysticks[id] then return 0 end
    return C.nxJoystickButtonCount(joysticks[id])
end

------------------------------------------------------------
function Joystick.getAxisCount(id)
    if not joysticks[id] then return 0 end
    return C.nxJoystickAxisCount(joysticks[id])
end

------------------------------------------------------------
function Joystick.getBallCount(id)
    if not joysticks[id] then return 0 end
    return C.nxJoystickBallCount(joysticks[id])
end

------------------------------------------------------------
function Joystick.getHatCount(id)
    if not joysticks[id] then return 0 end
    return C.nxJoystickHatCount(joysticks[id])
end

------------------------------------------------------------
function Joystick.isButtonDown(id, button)
    if not joysticks[id] then return false end
    return C.nxJoystickGetButton(joysticks[id], button)
end

------------------------------------------------------------
function Joystick.getAxisPosition(id, axis)
    if not joysticks[id] then return 0 end
    return tonumber(C.nxJoystickGetAxis(joysticks[id], axis))
end

------------------------------------------------------------
function Joystick.getBallPosition(id, ball)
    if not joysticks[id] then return 0, 0 end
    local posPtr = ffi.new('int[2]')
    C.nxJoystickGetBall(joysticks[id], ball, posPtr)
    return tonumber(posPtr[0]), tonumber(posPtr[1])
end

------------------------------------------------------------
function Joystick.getHatPosition(id, hat)
    if not joysticks[id] then return 'centered' end
    return hatPos[C.nxJoystickGetHat(joysticks[id], hat)]
end

------------------------------------------------------------
function Joystick.getName(id)
    if not joysticks[id] then return '' end
    return ffi.string(C.nxJoystickGetName(joysticks[id]))
end

------------------------------------------------------------
function Joystick.getGUID(id)
    if not joysticks[id] then return '' end
    return ffi.string(C.nxJoystickGetGUID(joysticks[id]))
end

-- Automatically called on joyconnect events ---------------
function Joystick.__connectEvent(id, isConnected)
    if isConnected then
        joysticks[id] = ffi.gc(C.nxJoystickOpen(id), C.nxJoystickClose)
    elseif joysticks[id] ~= nil then
        C.nxJoystickClose(ffi.gc(joysticks[id], nil))
        joysticks[id] = nil
    end
end

------------------------------------------------------------
return Joystick