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
local Window       = require 'window'
local Matrix       = require 'util.matrix'
local Entity3D     = require 'graphics.entity3d'
local Renderbuffer = require 'graphics.renderbuffer'

local Camera3D = Entity3D:subclass 'graphics.camera3d'

local ffi = require 'ffi'
local C   = ffi.C

function Camera3D:initialize(a, b, c, d, e, f)
    Entity3D.initialize(self)
    self:setViewport()
    if e then
        self:setOrtho(a, b, c, d, e, f)
    else
        self:setPerspective(a, b, c, d)
    end
end

function Camera3D:_invalidate()
    Entity3D._invalidate(self)
    self._projection = nil
    self._invProjection = nil

    return self
end

function Camera3D:setView(left, right, bottom, top, near, far)
    self._left, self._right, self._bottom, self._top, self._near, self._far =
        left, right, bottom, top, near, far

    return self:_invalidate()
end

function Camera3D:setPerspective(fov, aspect, near, far)
    fov, near, far = fov or 70, near or 1, far or -100
    if not aspect then
        local w, h = Window.size()
        aspect = w/h
    end

    self._fov, self._aspect, self._perspective = fov, aspect, true

    local ymax = near * math.tan(fov/360)
    local xmax = ymax * aspect
    return self:setView(-xmax, xmax, -ymax, ymax, near, far)
end

function Camera3D:setOrtho(left, right, bottom, top, near, far)
    self._fov, self._aspect, self._perspective = nil, nil, false

    if left then
        return self:setView(left, right, bottom, top, near or 1, far or -1)
    else
        local w, h = Window.size()
        return self:setView(-w/h, w/h, 1, -1, 1, -1)
    end
end

function Camera3D:setViewport(left, top, width, height)
    if not left then
        left, top, width, height = 0, 0, Window.size()
    elseif not width then
        left, top, width, height = 0, 0, left, top
    end

    self._vpX, self._vpY, self._vpW, self._vpH = left, top, width, height

    return self
end

function Camera3D:setRenderbuffer(rb)
    self._rb = rb

    return self
end

function Camera3D:viewport()
    return self._vpX, self._vpY, self._vpW, self._vpH
end

function Camera3D:renderbuffer()
    return self._rb
end

function Camera3D:view()
    return self._left, self._right, self._bottom, self._top, self._near, self._far
end

function Camera3D:isPerspective()
    return self._perspective, self._fov, self._aspect, self._near, self._far
end

function Camera3D:projection()
    if not self._projection then
        local func = self._perspective and Matrix.fromFrustum or Matrix.fromOrtho
        self._projection = func(
                self._left, self._right, self._bottom, self._top, self._near, self._far
            )
            :combine(self:matrix(true):inverse())
    end

    return self._projection
end

function Camera3D:invProjection()
    if not self._invProjection then
        self._invProjection = self:projection():inverse()
    end

    return self._invProjection
end

function Camera3D:clear(r, g, b, a, depth, col0, col1, col2, col3, clearDepth)
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

function Camera3D:apply()
    C.nxRendererSetViewport(self._vpX, self._vpY, self._vpW, self._vpH)
    Renderbuffer.bind(self._rb)

    return self
end

function Camera3D:_draw()
    -- Nullifier override / Nothing to do
end

function Camera3D:draw(drawable, context)
    self:apply()

    drawable:_draw(self, context or 'ambient')

    return self
end

return Camera3D
