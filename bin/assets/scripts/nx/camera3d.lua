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
local Entity3D    = require 'nx.entity3d'
local Camera      = require 'nx._camera'

local Camera3D = Camera:subclass('nx.camera3d')
Camera3D:include(Entity3D)

------------------------------------------------------------
function Camera3D:initialize(fov, aspect, near, far)
    self:reset(fov, aspect, near, far)
end

------------------------------------------------------------
function Camera3D:reset(fov, aspect, near, far)
    self._fov, self._aspect, self._near, self._far = fov, aspect, near, far
    Entity3D.initialize(self)

    return self
end

------------------------------------------------------------
function Camera3D:matrix()
    if not self._matrix then
        self._matrix = Matrix.fromPerspective(self._fov, self._aspect, self._near, self._far) 

        if self._targetX then
            self._matrix
                :combine(Matrix.fromLookAt(
                    self._originX-self._posX, self._originY-self._posY, self._originZ-self._posZ,
                    self._targetX, self._targetY, self._targetZ,
                    self._upX, self._upY, self._upZ
                ))
                :combine(Matrix.fromScaling(self._scaleX, self._scaleY, self._scaleZ))
        else
            self._matrix
                :combine(Matrix.fromTranslation(self._originX, self._originY, self._originZ))
                :combine(Matrix.fromScaling(1/self._scaleX, 1/self._scaleY, 1/self._scaleZ))
                :combine(Matrix.fromRotation(-self._rotX, -self._rotY, -self._rotZ))
                :combine(Matrix.fromTranslation(-self._posX, -self._posY, -self._posZ))
        end
    end

    return self._matrix
end

------------------------------------------------------------
function Camera3D:_draw()
    -- Nullifier override / Nothing to do
end

------------------------------------------------------------
function Camera3D:draw(drawable, state)
    -- Temporarily until we add 3d entities
    
    self:apply()

    state = state and state:clone() or RenderState:new()
    drawable:_draw(self, state)

    return self
end

------------------------------------------------------------
return Camera3D