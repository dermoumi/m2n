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
-- Represents a 2D view in space
------------------------------------------------------------
local class = require 'nx.class'
local Camera2D = class 'nx.camera2d'

local Matrix4 = require 'nx.matrix4'

------------------------------------------------------------
-- FFI declarations
------------------------------------------------------------
local ffi = require 'ffi'
ffi.cdef [[
    typedef struct {
        float centerX, centerY;
        float width, height;
        float rotation;
        float vpX, vpY, vpW, vpH;
    } NxCamera2D;
]]

------------------------------------------------------------
function Camera2D.static._fromCData(cdata)
    local camera = Camera2D:allocate()
    camera._cdata = ffi.cast('NxCamera*', cdata)
    return camera
end

------------------------------------------------------------
function Camera2D:initialize(x, y, width, height)
    self._cdata = ffi.new('NxCamera2D')
    self:reset(x, y, width, height)
end

------------------------------------------------------------
function Camera2D:setCenter(x, y)
    local c = self._cdata

    c.centerX = x
    c.centerY = y

    self._matrix = nil
    self._invMatrix = nil
end

------------------------------------------------------------
function Camera2D:setSize(width, height)
    local c = self._cdata

    c.width = width
    c.height = height

    self._matrix = nil
    self._invMatrix = nil
end

------------------------------------------------------------
function Camera2D:setRotation(rad)
    local c = self._cdata

    c.rotation = rad % (math.pi * 2)

    self._matrix = nil
    self._invMatrix = nil
end

------------------------------------------------------------
function Camera2D:setViewport(x, y, width, height)
    local c = self._cdata

    c.vpX = x
    c.vpY = y
    c.vpW = width
    c.vpH = height
end

------------------------------------------------------------
function Camera2D:reset(x, y, width, height)
    local c = self._cdata

    x = x or 0
    y = y or 0
    width  = width  or 1
    height = height or 1

    c.centerX  = x + width / 2
    c.centerY  = y + height / 2
    c.width    = width
    c.height   = height
    c.rotation = 0

    self._matrix = nil
    self._invMatrix = nil
end

------------------------------------------------------------
function Camera2D:center()
    local c = self._cdata
    return c.centerX, c.centerY
end

------------------------------------------------------------
function Camera2D:size()
    local c = self._cdata
    return c.width, c.height
end

------------------------------------------------------------
function Camera2D:rotation()
    local c = self._cdata
    return c.rotation
end

------------------------------------------------------------
function Camera2D:viewport()
    local c = self._cdata
    return c.vpX, c.vpY, c.vpW, c.vpH
end

------------------------------------------------------------
function Camera2D:zoom(factor, factor2)
    local w, h = self:size()
    self:setSize(w * factor, h * factor2)
end

------------------------------------------------------------
function Camera2D:matrix()
    if not self._matrix then
        local c = self._cdata

        local cos = math.cos(c.rotation)
        local sin = math.sin(c.rotation)
        local tx  = c.centerX - c.centerX * cos - c.centerY * sin
        local ty  = c.centerY + c.centerX * sin - c.centerY * cos

        -- Projection components
        local x =  2 / c.width
        local y = -2 / c.height
        local z = -x * c.centerX
        local w = -y * c.centerY

        -- Rebuild the projection matrix
        self._matrix = Matrix4:new()
        local m = self._matrix._cdata
        m[0], m[1], m[3] =  x * cos,     x * sin, x * tx + z
        m[4], m[5], m[7] = -y * sin, y * cos, y * ty + w
    end

    return self._matrix
end

------------------------------------------------------------
function Camera2D:invMatrix()
    if not self._invMatrix then
        self._invMatrix = self:matrix():inverse()
    end

    return self._invMatrix
end

------------------------------------------------------------
return Camera2D