--[[
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
--]]

local Graphics     = require 'graphics'
local Renderbuffer = require 'graphics.renderbuffer'
local Matrix       = require 'util.matrix'
local Window       = require 'window'
local class        = require 'class'

local Camera2D = class 'graphics.camera2d'

local ffi = require 'ffi'
local C   = ffi.C

function Camera2D:initialize(x, y, width, height)
    self:setViewport()
    self:reset(x, y, width, height)
end

function Camera2D:_markDirty()
    self._projection = nil
    self._invProjection = nil

    return self
end

function Camera2D:setCenter(x, y)
    self._centerX = x
    self._centerY = y

    return self:_markDirty()
end

function Camera2D:setSize(width, height)
    self._width = width
    self._height = height

    return self:_markDirty()
end

function Camera2D:setRotation(rad)
    self._rotation = rad % (math.pi * 2)
    if self._rotation < 0 then self._rotation = self._rotation + math.pi * 2 end

    return self:_markDirty()
end

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

    return self:_markDirty()
end

function Camera2D:center()
    return self._centerX, self._centerY
end

function Camera2D:size()
    return self._width, self._height
end

function Camera2D:rotation()
    return self._rotation
end

function Camera2D:zoom(factor, factor2)
    local w, h = self:size()
    self:setSize(w * factor, h * factor2)
end

function Camera2D:setViewport(left, top, width, height)
    if not left then
        left, top, width, height = 0, 0, Window.size()
    elseif not width then
        left, top, width, height = 0, 0, left, top
    end

    self._vpX, self._vpY, self._vpW, self._vpH = left, top, width, height

    return self
end

function Camera2D:setRenderbuffer(rb)
    self._rb = rb

    return self
end

function Camera2D:viewport()
    return self._vpX, self._vpY, self._vpW, self._vpH
end

function Camera2D:renderbuffer()
    return self._rb
end

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

function Camera2D:invProjection()
    if not self._invProjection then
        self._invProjection = self:projection():inverse()
    end

    return self._invProjection
end

function Camera2D:draw(drawable, context)
    self:apply()

    drawable:_draw(self, context)

    return self
end

function Camera2D:drawFsQuad(texture, width, height)
    self:apply()
    Graphics.drawFsQuad(texture, width, height)

    return self
end

function Camera2D:fillFsQuad(r, g, b, a, blendMode)
    self:apply()
    Graphics.fillFsQuad(r, g, b, a, blendMode)

    return self
end

function Camera2D:clear(r, g, b, a, depth, col0, col1, col2, col3, clearDepth)
    self:apply()

    -- Make sure the values are valid
    if r == nil then r, g, b = 0, 0, 0 end
    if col0 == nil then col0, col1, col2, col3 = true, true, true, true end
    if clearDepth == nil then clearDepth = true end

    C.nxRendererClear(
        r, g, b, a or 0, depth or 1.0, col0, col1, col2, col3, clearDepth
    )

    return self
end

function Camera2D:apply()
    C.nxRendererSetViewport(self._vpX, self._vpY, self._vpW, self._vpH)
    Renderbuffer.bind(self._rb)

    Graphics.enableDepthTest(false)
        .enableDepthMask(false)

    return self
end

return Camera2D
