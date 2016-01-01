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

local Keyboard  = require 'nx.window.keyboard'
local Mouse     = require 'nx.window.mouse'
local Window    = require 'nx.window'
local Geometry  = require 'nx.graphics.geometry'
local ModelNode = require 'nx.graphics.modelnode'
local Model     = require 'nx.graphics.model'
local Material  = require 'nx.graphics.material'
local Entity3D  = require 'nx.graphics.entity3d'
local Scene     = require 'scene'

local SceneTest3D = Scene:subclass('scene.test.3d')

local Node = require('nx.class')('node')
Node:include(Entity3D)

--------------------------------------------------------
function SceneTest3D:load()
    self.player = Node:new()
        -- :setPosition(0, 0, 3)

    self.camera = require('nx.graphics.camera3d'):new()
        :setParent(self.player)
        -- :setRotation(0, 0, math.pi / 4)
        -- :setScaling(2, 2, 2)
        -- :lookAt(0, 0, 0)

    local cubeGeom = Geometry:new()
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

    local solidOrangeMat = Material:new()
        :setColor(255, 128, 0)

    local cubeModel = Model:new()
        :setGeometry(cubeGeom)
        :addMesh(nil, 0, cubeGeom:vertexCount())

    local orangeCubeModel = Model:new()
        :setGeometry(cubeGeom)
        :addMesh(solidOrangeMat, 0, cubeGeom:vertexCount())

    self.mesh = ModelNode:new()
        :setModel(orangeCubeModel)
        :setPosition(0, 0, -3)
    
    self.subMesh = ModelNode:new()
        :setParent(self.mesh)
        :setModel(cubeModel)
        :setPosition(0, 1, 0)
        :setScaling(.5, .5, .5)

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
    -- self.subMesh:rotate(0, 0, math.pi * dt / 2)
    self.subMesh:rotate(0, math.pi * dt / 2, 0)

    if self.camVelX ~= 0 or self.camVelY ~= 0 or self.camVelZ ~= 0 then
        local q = self.player:quaternion()
        self.player:move(q:apply(self.camVelX * dt, self.camVelY * dt, self.camVelZ * dt))
    end
end

------------------------------------------------------------
function SceneTest3D:render()
    require('nx.graphics')
        .setDepthFunc('lequal')
        .enableDepthTest(true)
        .enableDepthMask(true)
        -- .setFillMode('wireframe')

    self.camera:clear(200, 200, 200)

    self.camera:draw(self.mesh)
end

------------------------------------------------------------
function SceneTest3D:onKeyDown(scancode, keyCode, repeated)
    if repeated then return end

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
    self.player:rotate(0, -xRel * self.camSensitivity, 0)
    self.camera:rotate(-yRel * self.camSensitivity, 0, 0)
end

------------------------------------------------------------
function SceneTest3D:updateParent()
    return self.parent and self.parent.class.name == 'scene.title'
end

------------------------------------------------------------
return SceneTest3D