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

local Window   = require 'nx.window'
local Keyboard = require 'nx.keyboard'
local Mouse    = require 'nx.mouse'
local Mesh     = require 'nx.mesh'
local Scene    = require 'scene'

local SceneTest3D = Scene:subclass('scene.test.3d')

--------------------------------------------------------
function SceneTest3D:load()
    self.camera = require('nx.camera3d')
        :new(70, 16/9, 1, -1000)
        :setPosition(0, 0, 3)
        -- :setRotation(0, 0, math.pi / 4)
        -- :setScaling(2, 2, 2)
        -- :lookAt(0, 0, 0)

    self.mesh = Mesh:new()
        :setVertexData(
            -1,-1,-1, 0, 0,
            -1,-1, 1, 0, 0,
            -1, 1, 1, 0, 0,
             1, 1,-1, 0, 0,
            -1,-1,-1, 0, 0,
            -1, 1,-1, 0, 0,
             1,-1, 1, 0, 0,
            -1,-1,-1, 0, 0,
             1,-1,-1, 0, 0,
             1, 1,-1, 0, 0,
             1,-1,-1, 0, 0,
            -1,-1,-1, 0, 0,
            -1,-1,-1, 0, 0,
            -1, 1, 1, 0, 0,
            -1, 1,-1, 0, 0,
             1,-1, 1, 0, 0,
            -1,-1, 1, 0, 0,
            -1,-1,-1, 0, 0,
            -1, 1, 1, 0, 0,
            -1,-1, 1, 0, 0,
             1,-1, 1, 0, 0,
             1, 1, 1, 0, 0,
             1,-1,-1, 0, 0,
             1, 1,-1, 0, 0,
             1,-1,-1, 0, 0,
             1, 1, 1, 0, 0,
             1,-1, 1, 0, 0,
             1, 1, 1, 0, 0,
             1, 1,-1, 0, 0,
            -1, 1,-1, 0, 0,
             1, 1, 1, 0, 0,
            -1, 1,-1, 0, 0,
            -1, 1, 1, 0, 0,
             1, 1, 1, 0, 0,
            -1, 1, 1, 0, 0,
             1,-1, 1, 0, 0
        )
    
    self.subMesh = self.mesh:clone(true)
        :setPosition(0, 1, 0)
        :setScaling(.5, .5, .5)
        :setParent(self.mesh)

    self.mesh:material():setColor(255, 128, 0)

    self.camVelX, self.camVelY, self.camVelZ, self.camSpeed = 0, 0, 0, 3
    self.camSensitivity = 0.001

    self.prevMouseMode = Mouse.isRelativeMode()
    Mouse.setRelativeMode(true)
end

------------------------------------------------------------
function SceneTest3D:release()
    Mouse.setRelativeMode(self.prevMouseMode)
end

------------------------------------------------------------
function SceneTest3D:update(dt)
    local xRot, yRot, zRot = self.camera:rotation()

    self.camera:move(
        dt * (math.cos(yRot) * self.camVelX + math.sin(yRot) * self.camVelZ),
        self.camVelY * dt,
        dt * (math.cos(yRot) * self.camVelZ - math.sin(yRot) * self.camVelX)
    )
end

------------------------------------------------------------
function SceneTest3D:render()
    require('nx.renderer')
        .setDepthFunc('lequal')
        .enableDepthTest(true)
        .enableDepthMask(true)
        -- .setFillMode('wireframe')

    self.camera:clear(200, 200, 200)

    self.camera:draw(self.mesh)
end

------------------------------------------------------------
function SceneTest3D:onKeyDown(scancode, keyCode, repeated)
    if scancode == '1' then
        self:performTransition(Scene.back)
    elseif scancode == 'w' then
        self.camVelZ = -self.camSpeed
    elseif scancode == 'a' then
        self.camVelX = -self.camSpeed
    elseif scancode == 'left ctrl' then
        self.camVelY = -self.camSpeed
    elseif scancode == 's' then
        self.camVelZ = self.camSpeed 
    elseif scancode == 'd' then
        self.camVelX = self.camSpeed
    elseif scancode == 'space' then
        self.camVelY = self.camSpeed
    end
end

------------------------------------------------------------
function SceneTest3D:onKeyUp(scancode, keyCode)
    if scancode == 'a' then
        self.camVelX = Keyboard.scancodeDown('d') and self.camSpeed or 0
    elseif scancode == 'w' then
        self.camVelZ = Keyboard.scancodeDown('s') and self.camSpeed or 0
    elseif scancode == 'left ctrl' then
        self.camVelY = Keyboard.scancodeDown('space') and self.camSpeed or 0
    elseif scancode == 'd' then
        self.camVelX = Keyboard.scancodeDown('a') and -self.camSpeed or 0
    elseif scancode == 's' then
        self.camVelZ = Keyboard.scancodeDown('w') and -self.camSpeed or 0
    elseif scancode == 'space' then
        self.camVelY = Keyboard.scancodeDown('left ctrl') and -self.camSpeed or 0
    end
end

------------------------------------------------------------
function SceneTest3D:onMouseMotion(x, y, xRel, yRel)
    self.camera:rotate(-yRel * self.camSensitivity, -xRel * self.camSensitivity, 0)
end

------------------------------------------------------------
function SceneTest3D:updateParent()
    return self.parent and self.parent.class.name == 'scene.title'
end

------------------------------------------------------------
return SceneTest3D