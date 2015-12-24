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

local RenderState = require 'nx.renderstate'
local Matrix      = require 'nx.matrix'
local Renderer    = require 'nx.renderer'
local Window      = require 'nx.window'
local Camera      = require 'nx._camera'

local Camera2D = Camera:subclass('nx.camera2d')

------------------------------------------------------------
function Camera2D:initialize(x, y, width, height)
    self:reset(x, y, width, height)
end

------------------------------------------------------------
function Camera2D:setCenter(x, y)
    self._centerX = x
    self._centerY = y

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Camera2D:setSize(width, height)
    self._width = width
    self._height = height

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Camera2D:setRotation(rad)
    self._rotation = rad % (math.pi * 2)
    if self._rotation < 0 then self._rotation = self._rotation + math.pi * 2 end

    self._matrix = nil
    self._invMatrix = nil

    return self
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

    self._matrix    = nil
    self._invMatrix = nil

    return self
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
function Camera2D:matrix()
    if not self._matrix then
        local cos = math.cos(self._rotation)
        local sin = math.sin(self._rotation)
        local tx  = self._centerX - self._centerX * cos - self._centerY * sin
        local ty  = self._centerY + self._centerX * sin - self._centerY * cos

        -- Projection components
        local x =  2 / self._width
        local y = -2 / self._height

        -- Rebuild the projection matrix
        self._matrix = Matrix:new()
        local m = self._matrix._cdata
        m[0], m[4], m[12] =  x * cos, x * sin, x * tx - x * self._centerX
        m[1], m[5], m[13] = -y * sin, y * cos, y * ty - y * self._centerY
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
function Camera2D:draw(drawable, state)
    self:apply()

    state = state and state:clone() or RenderState:new()
    drawable:_draw(self, state)

    return self
end

------------------------------------------------------------
function Camera2D:drawFsQuad(texture, width, height)
    self:apply()
    Renderer.drawFsQuad(texture, width, height)

    return self
end

------------------------------------------------------------
function Camera2D:fillFsQuad(r, g, b, a, blendMode)
    self:apply()
    Renderer.fillFsQuad(r, g, b, a, blendMode)

    return self
end

------------------------------------------------------------
return Camera2D