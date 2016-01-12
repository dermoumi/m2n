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
local SceneEntity  = require 'graphics.sceneentity'
local Renderbuffer = require 'graphics.renderbuffer'

local CameraEntity = SceneEntity:subclass 'graphics.cameraentity'

local ffi = require 'ffi'
local C   = ffi.C

function CameraEntity:initialize(a, b, c, d, e, f)
    SceneEntity.initialize(self, 'camera')
    self:setViewport()

    if e then
        self:setOrtho(a, b, c, d, e, f)
    else
        self:setPerspective(a, b, c, d)
    end
end

function CameraEntity:_markDirty()
    SceneEntity._markDirty(self)
    self._projection = nil
    self._invProjection = nil

    return self
end

function CameraEntity:setView(left, right, bottom, top, near, far)
    self._left, self._right, self._bottom, self._top, self._near, self._far =
        left, right, bottom, top, near, far

    return self:_markDirty()
end

function CameraEntity:setPerspective(fov, aspect, near, far)
    fov, near, far = fov or 70, near or 0.1, far or -100
    if not aspect then
        local w, h = Window.size()
        aspect = w/h
    end

    self._fov, self._aspect, self._perspective = fov, aspect, true

    local ymax = near * math.tan(fov/180)
    local xmax = ymax * aspect
    return self:setView(-xmax, xmax, -ymax, ymax, near, far)
end

function CameraEntity:setOrtho(left, right, bottom, top, near, far)
    self._fov, self._aspect, self._perspective = nil, nil, false

    if left then
        return self:setView(left, right, bottom, top, near or 1, far or -1)
    else
        local w, h = Window.size()
        return self:setView(-w/h, w/h, 1, -1, 1, -1)
    end
end

function CameraEntity:setViewport(left, top, width, height)
    if not left then
        left, top, width, height = 0, 0, Window.size()
    elseif not width then
        left, top, width, height = 0, 0, left, top
    end

    self._vpX, self._vpY, self._vpW, self._vpH = left, top, width, height

    return self
end

function CameraEntity:setRenderbuffer(rb)
    self._rb = rb

    return self
end

function CameraEntity:viewport()
    return self._vpX, self._vpY, self._vpW, self._vpH
end

function CameraEntity:renderbuffer()
    return self._rb
end

function CameraEntity:view()
    return self._left, self._right, self._bottom, self._top, self._near, self._far
end

function CameraEntity:isPerspective()
    return self._perspective, self._fov, self._aspect, self._near, self._far
end

function CameraEntity:projection()
    if not self._projection then
        local func = self._perspective and Matrix.fromFrustum or Matrix.fromOrtho
        self._projection = func(
                self._left, self._right, self._bottom, self._top, self._near, self._far
            )
            :combine(self:matrix(true):inverse())
    end

    return self._projection
end

function CameraEntity:invProjection()
    if not self._invProjection then
        self._invProjection = self:projection():inverse()
    end

    return self._invProjection
end

function CameraEntity:clear(r, g, b, a, depth, col0, col1, col2, col3, clearDepth)
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

function CameraEntity:apply()
    C.nxRendererSetViewport(self._vpX, self._vpY, self._vpW, self._vpH)
    Renderbuffer.bind(self._rb)

    return self
end

function CameraEntity:draw(drawable, context)
    self:apply()

    drawable:_draw(self, context)

    return self
end

return CameraEntity
