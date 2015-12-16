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
-- Represents a Camera in a 3D space
------------------------------------------------------------
local Camera = require 'nx._camera'
local Camera3D = Camera:subclass('nx.camera3d')

local Matrix = require 'nx.matrix'

------------------------------------------------------------
local ffi = require 'ffi'
ffi.cdef [[
    typedef struct {
        float fov, aspect, near, far;
        float posX, posY, posZ;
        float rotX, rotY, rotZ;
        float scaleX, scaleY, scaleZ;
        float originX, originY, originZ;
    } NxCamera3D;
]]

------------------------------------------------------------
function Camera3D.static._fromCData(data)
    local camera = Camera3D:allocate()
    camera._cdata = ffi.cast('NxCamera3D*', data)
    return camera
end

------------------------------------------------------------
function Camera3D:initialize(fov, aspect, near, far)
    self._cdata = ffi.new('NxCamera3D')
    self:reset(fov, aspect, near, far)
end
------------------------------------------------------------
function Camera3D:reset(fov, aspect, near, far)
    local c = self._cdata

    c.fov, c.aspect, c.near, c.far = fov, aspect, near, far
    c.posX, c.posY, c.posZ = 0, 0, 0
    c.rotX, c.rotY, c.rotZ = 0, 0, 0
    c.scaleX, c.scaleY, c.scaleZ = 1, 1, 1
    c.originX, c.originY, c.originZ = 0, 0, 0

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Camera3D:setPosition(x, y, z)
    local c = self._cdata
    c.posX, c.posY, c.posZ = x, y, z

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Camera3D:setScaling(x, y, z)
    local c = self._cdata
    c.scaleX, c.scaleY, c.scaleZ = x, y, z

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Camera3D:setRotation(x, y, z)
    local pi2 = math.pi * 2

    local c = self._cdata
    c.rotX, c.rotY, c.rotZ = x % pi2, y % pi2, z % pi2
    if c.rotX < 0 then c.rotX = c.rotX + pi2 end
    if c.rotY < 0 then c.rotY = c.rotY + pi2 end
    if c.rotZ < 0 then c.rotZ = c.rotZ + pi2 end

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Camera3D:setOrigin(x, y, z)
    local c = self._cdata
    c.originX, c.originY, c.originZ = x, y, z

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Camera3D:translate(x, y, z)
    local c = self._cdata
    return self:setPosition(c.posX + x, c.posY + y, c.posZ + z)
end

------------------------------------------------------------
function Camera3D:scale(x, y, z)
    local c = self._cdata
    return self:setScaling(c.scaleX * x, c.scaleY * y, c.scaleZ * z)
end

------------------------------------------------------------
function Camera3D:rotate(x, y, z)
    local c = self._cdata
    return self:setRotation(c.rotX + x, c.rotY + y, c.rotZ + z)
end

------------------------------------------------------------
function Camera3D:offset(x, y, z)
    local c = self._cdata
    return self:setOrigin(c.originX + x, c.originY + y, c.originZ + z)
end

------------------------------------------------------------
function Camera3D:position()
    local c = self._cdata
    return c.posX, c.posY, c.posZ
end

------------------------------------------------------------
function Camera3D:scaling()
    local c = self._cdata
    return c.scaleX, c.scaleY, c.scaleZ
end

------------------------------------------------------------
function Camera3D:rotation()
    local c = self._cdata
    return c.rotX, c.rotY, c.rotZ
end

------------------------------------------------------------
function Camera3D:origin()
    local c = self._cdata
    return c.originX, c.originY, c.originZ
end

------------------------------------------------------------
function Camera3D:matrix()
    if not self._matrix then
        local c = self._cdata
        self._matrix = Matrix.fromPerspective(c.fov, c.aspect, c.near, c.far)
            :combine(Matrix.fromTranslation(c.posX, c.posY, c.posZ))
            :combine(Matrix.fromRotation(c.rotX, c.rotY, c.rotZ))
            :combine(Matrix.fromScaling(c.scaleX, c.scaleY, c.scaleZ))
            :combine(Matrix.fromTranslation(-c.originX, -c.originY, -c.originZ))
    end

    return self._matrix
end

------------------------------------------------------------
function Camera3D:invMatrix()
    if not self._invMatrix then
        self._invMatrix = self._matrix:inverse()
    end

    return self._invMatrix
end

------------------------------------------------------------
function Camera3D:draw(drawable, state)
    -- Temporarily until we add 3d entities
    
    self:apply()

    state = state and state:clone() or require('nx.entity2d').State:new()
    drawable:_draw(self, state)

    return self
end

------------------------------------------------------------
return Camera3D