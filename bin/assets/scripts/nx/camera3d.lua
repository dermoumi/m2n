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

    c.fov = fov
    c.aspect = aspect
    c.near = near
    c.far = far

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Camera3D:translate(x, y, z)
    self:matrix():combine(Matrix.fromTranslation(x, y, z))
end

------------------------------------------------------------
function Camera3D:matrix()
    if not self._matrix then
        local c = self._cdata
        self._matrix = Matrix.fromPerspective(c.fov, c.aspect, c.near, c.far)
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
    self:apply()

    state = state and state:clone() or require('nx.entity2d').State:new()
    drawable:_draw(self, state)

    return self
end

------------------------------------------------------------
return Camera3D