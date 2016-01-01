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

local class = require 'class'

local Quaternion = class 'util.quaternion'

------------------------------------------------------------
local EPSILON = 0.000001

------------------------------------------------------------
function Quaternion.static.fromToRotation(x1, y1, z1, x2, y2, z2)
    local dot = x1*x2 + y1*y2 + z1*z2
    if dot <= -1 then
        -- Cross product with (1, 0, 0)
        local x, y, z = 0, -z1, y1
        -- If 0, cross product with (0, 1, 0)
        if math.sqrt(y*y+z*z) < EPSILON then x, y, z = z1, 0, -x1 end
        -- Normalize
        local len = x*x + y*y + z*z
        if len ~= 0 then
            len = math.sqrt(len)
            x, y, z = x/len, y/len, z/len
        end
        -- Axis angle
        return Quaternion.fromAxisAngle(x, y, z, math.pi)
    elseif dot >= 1 then
        -- Invalid?
        return Quaternion:new(0, 0, 0, 1)
    else
        -- Cross product a*b
        local x, y, z = y1*z2 - z1*y2, z1*x2 - x1*z2, x1*y2 - y1*x2
        return Quaternion:new(x, y, z, 1 + dot):normalize()
    end
end

------------------------------------------------------------
function Quaternion.static.fromAxisAngle(x, y, z, rad)
    rad = rad / 2
    local s = math.sin(rad)
    return Quaternion:new(s*x, s*y, s*z, math.cos(rad))
end

------------------------------------------------------------
function Quaternion.static.combine(quat1, quat2)
    return Quaternion:new(quat1):combine(quat2)
end

------------------------------------------------------------
function Quaternion:initialize(x, y, z, w)
    if not x then
        self.x, self.y, self.z, self.w = 0, 0, 0, 0
    elseif not y then
        self.x, self.y, self.z, self.w = x.x, x.y, x.z, x.w
    elseif w then
        self.x, self.y, self.z, self.w = x, y, z, w
    else
        self.x, self.y, self.z, self.w = 0, math.sin(y/2), 0, math.cos(y/2)

        self:combine(Quaternion:new(math.sin(x/2), 0, 0, math.cos(x/2)))
            :combine(Quaternion:new(0, 0, math.sin(z/2), math.cos(z/2)))
    end
end

------------------------------------------------------------
function Quaternion:combine(q)
    local x, y, z, w = self.x, self.y, self.z, self.w

    self.x, self.y, self.z, self.w =
        y * q.z - z * q.y + q.x * w + x * q.w,
        z * q.x - x * q.z + q.y * w + y * q.w,
        x * q.y - y * q.x + q.z * w + z * q.w,
        w * q.w - x * q.x - y * q.y - z * q.z

    return self
end

------------------------------------------------------------
function Quaternion:angles()
    self:normalize()

    local wx, wy, wz, xx, yy, zz, xy, xz, yz =
        self.w * self.x, self.w * self.y, self.w * self.z,
        self.x * self.x, self.y * self.y, self.z * self.z,
        self.x * self.y, self.x * self.z, self.y * self.z

    return math.atan2(2 * (wx+yz), 1 - 2 * (xx+yy)),
        math.asin(2 * (wy-xz)),
        math.atan2(2 * (wz+xy), 1 - 2 * (yy+zz))
end

------------------------------------------------------------
function Quaternion:normalize()
    local len = self.x*self.x + self.y*self.y + self.z*self.z + self.w*self.w

    if len ~= 0 then
        len = math.sqrt(len)
        self.x, self.y, self.z, self.w = self.x/len, self.y/len, self.z/len, self.w/len
    end

    return self
end

------------------------------------------------------------
function Quaternion:apply(x, y, z)
    local x2, y2, z2 = self.x * 2,  self.y * 2,  self.z * 2
    local a1, a2, a3 = self.x * x2, self.y * y2, self.z * z2
    local a4, a5, a6 = self.x * y2, self.x * z2, self.y * z2
    local a7, a8, a9 = self.w * x2, self.w * y2, self.w * z2

    return (1 - a2 - a3) * x + (a4 - a9) * y + (a5 + a8) * z,
        (a4 + a9) * x + (1 - a1 - a3) * y + (a6 - a7) * z,
        (a5 - a8) * x + (a6 + a7) * y + (1 - a1 - a2) * z
end

------------------------------------------------------------
return Quaternion