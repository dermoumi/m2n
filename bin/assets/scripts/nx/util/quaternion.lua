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

local class = require 'nx.class'

local Quaternion = class 'nx.util.quaternion'

------------------------------------------------------------
function Quaternion:initialize(x, y, z, w)
    if not x then
        self.x, self.y, self.z, self.w = 0, 0, 0, 0
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
    local wx, wy, wz, xx, yy, zz, xy, xz, yz =
        self.w * self.x, self.w * self.y, self.w * self.z,
        self.x * self.x, self.y * self.y, self.z * self.z,
        self.x * self.y, self.x * self.z, self.y * self.z

    return math.atan2(2 * (wx+yz), 1 - 2 * (xx+yy)),
        math.asin(2 * (wy-xz)),
        math.atan2(2 * (wz+xy), 1 - 2 * (yy+zz))
end

------------------------------------------------------------
function Quaternion:clone()
    return Quaternion:new(self.x, self.y, self.z, self.w)
end

------------------------------------------------------------
return Quaternion