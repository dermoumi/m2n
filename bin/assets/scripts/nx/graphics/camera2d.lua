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

local Graphics    = require 'nx.graphics'
local Matrix      = require 'nx.graphics.matrix'
local Window      = require 'nx.window'
local Camera      = require 'nx.graphics._camera'

local Camera2D = Camera:subclass('nx.graphics.camera2d')

------------------------------------------------------------
function Camera2D:initialize(x, y, width, height)
    self:reset(x, y, width, height)
end

------------------------------------------------------------
function Camera2D:setCenter(x, y)
    self._centerX = x
    self._centerY = y

    return self:_invalidate()
end

------------------------------------------------------------
function Camera2D:setSize(width, height)
    self._width = width
    self._height = height

    return self:_invalidate()
end

------------------------------------------------------------
function Camera2D:setRotation(rad)
    self._rotation = rad % (math.pi * 2)
    if self._rotation < 0 then self._rotation = self._rotation + math.pi * 2 end

    return self:_invalidate()
end

------------------------------------------------------------
function Camera2D:reset(x, y, width, height)
    local winWidth, winHeight = Window:size()

    x = x or 0
    y = y or 0
    width  = width  or winWidth
    height = height or winHeight

    self._centerX  = x + width / 2
    self._centerY  = y + height / 2
    self._width    = width
    self._height   = height
    self._rotation = 0

    return self:_invalidate()
end

------------------------------------------------------------
function Camera2D:center()
    return self._centerX, self._centerY
end

------------------------------------------------------------
function Camera2D:size()
    return self._width, self._height
end

------------------------------------------------------------
function Camera2D:rotation()
    return self._rotation
end

------------------------------------------------------------
function Camera2D:zoom(factor, factor2)
    local w, h = self:size()
    self:setSize(w * factor, h * factor2)
end

------------------------------------------------------------
function Camera2D:projection()
    if not self._projection then
        -- Projection components
        local cos, sin =
            math.cos(self._rotation),
            math.sin(self._rotation)
        local tx, ty, x, y =
            self._centerX - self._centerX * cos - self._centerY * sin,
            self._centerY + self._centerX * sin - self._centerY * cos,
            2 / self._width,
            -2 / self._height

        -- Rebuild the projection matrix
        self._projection = Matrix:new()
        local m = self._projection._cdata
        m[0], m[4], m[12] =  x * cos, x * sin, x * tx - x * self._centerX
        m[1], m[5], m[13] = -y * sin, y * cos, y * ty - y * self._centerY
    end

    return self._projection
end

------------------------------------------------------------
function Camera2D:draw(drawable, context)
    self:apply()

    drawable:_draw(self, context)

    return self
end

------------------------------------------------------------
function Camera2D:drawFsQuad(texture, width, height)
    self:apply()
    Graphics.drawFsQuad(texture, width, height)

    return self
end

------------------------------------------------------------
function Camera2D:fillFsQuad(r, g, b, a, blendMode)
    self:apply()
    Graphics.fillFsQuad(r, g, b, a, blendMode)

    return self
end

------------------------------------------------------------
function Camera2D:apply()
    Camera.apply(self)

    Graphics.enableDepthTest(false)
        .enableDepthMask(false)
end

------------------------------------------------------------
return Camera2D