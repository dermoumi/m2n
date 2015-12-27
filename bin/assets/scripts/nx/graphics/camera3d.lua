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

local Matrix      = require 'nx.graphics.matrix'
local Window      = require 'nx.window'
local Entity3D    = require 'nx.graphics.entity3d'
local Camera      = require 'nx.graphics._camera'

local Camera3D = Camera:subclass('nx.graphics.camera3d')
Camera3D:include(Entity3D)

------------------------------------------------------------
function Camera3D:initialize(fov, aspect, near, far)
    self:setPerspective(fov, aspect, near, far)
    Entity3D.initialize(self)
end

------------------------------------------------------------
function Camera3D:_invalidate()
    Entity3D._invalidate(self)
    Camera._invalidate(self)

    return self
end

------------------------------------------------------------
function Camera3D:setView(type, a, b, c, d, e, f)
    self._type, self._a, self._b, self._c, self._d, self._e, self._f = type, a, b, c, d, e, f

    return self:_invalidate()
end

------------------------------------------------------------
function Camera3D:setPerspective(fov, aspect, near, far)
    local w, h = Window.size()
    return self:setView('perspective', fov or 70, aspect or w/h, near or 1, far or -1000)
end

------------------------------------------------------------
function Camera3D:setOrtho(left, right, bottom, top, near, far)
    if left then
        return self:setView('ortho', left, right, bottom, top, near or 1, far or -1000)
    else
        local w, h = Window.size()
        return self:setView('ortho', -w/h, w/h, 1, -1, 1, -1000)
    end
end

------------------------------------------------------------
function Camera3D:setFrustum(left, right, bottom, top, near, far)
    if left then
        return self:setView('frustum', left, right, bottom, top, near or 1, far or -1000)
    else
        local w, h = Window.size()
        return self:setView('frustum', -w/h, w/h, 1, -1, 1, -1000)
    end
end

------------------------------------------------------------
function Camera3D:view()
    return self._type, self._a, self._b, self._c, self._d, self._e, self._f
end

------------------------------------------------------------
function Camera3D:projection()
    if not self._projection then
        if self._type == 'perspective' then
            self._projection = Matrix.fromPerspective(self._a, self._b, self._c, self._d)
        elseif self._type == 'ortho' then
            self._projection = Matrix.fromOrtho(
                self._a, self._b, self._c, self._d, self._e, self._f
            )
        elseif self._type == 'frustum' then
            self._projection = Matrix.fromFrustum(
                self._a, self._b, self._c, self._d, self._e, self._f
            )
        end

        if self._targetX then
            self._projection
                :combine(Matrix.fromLookAt(
                    self._originX-self._posX, self._originY-self._posY, self._originZ-self._posZ,
                    self._targetX, self._targetY, self._targetZ,
                    self._upX, self._upY, self._upZ
                ))
                :combine(Matrix.fromScaling(self._scaleX, self._scaleY, self._scaleZ))
        else
            self._projection:combine(self:matrix():inverse())
        end
    end

    return self._projection
end

------------------------------------------------------------
function Camera3D:_draw()
    -- Nullifier override / Nothing to do
end

------------------------------------------------------------
function Camera3D:draw(drawable, context)
    self:apply()

    drawable:_draw(self, context or 'ambient')

    return self
end

------------------------------------------------------------
return Camera3D