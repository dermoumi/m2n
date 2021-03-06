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

local Keyboard     = require 'window.keyboard'
local Mouse        = require 'window.mouse'
local Window       = require 'window'
local Graphics     = require 'graphics'
local Geometry     = require 'graphics.geometry'
local Material     = require 'graphics.material'
local Entity3D     = require 'graphics.entity3d'
local Screen       = require 'screen'
local Scene        = require 'graphics.scene'
local RenderBuffer = require 'graphics.renderbuffer'
local Util         = require 'util'

local ScreenTest3D = Screen:subclass 'screen.test.3d'

function ScreenTest3D:initialize()
    self.sceneDesc = self:cache('scene:assets/scenes/Scene.scene')
    self.depthShader = self:cache('shader:assets/shaders/outline.shader')
end

function ScreenTest3D:entered()
    self.text = require('graphics.text')
        :new('', require 'game.font', 14)
        :setPosition(10, 10)

    self.scene = Scene:new(self.sceneDesc)

    self.player = self.scene:lookupName('player') or Scene:new()

    self.camera = self.scene:lookupName('main_camera') or require('graphics.camera'):new()
        :attachTo('main_camera', self.player)

    self.cube = (self.scene:lookupName('cube') or require('graphics.model'):new()):detach()
    -- self.cube:setPosition(0, 0, -3)

    self.subMesh = self.cube:lookupName('sphere') or require('graphics.model'):new()

    self.camVelX, self.camVelY, self.camVelZ, self.camSpeed = 0, 0, 0, 4
    self.camSensitivity = 0.001

    self.prevMouseMode = Mouse.isRelativeMode()
    Mouse.setRelativeMode(true)
end

function ScreenTest3D:left()
    Mouse.setRelativeMode(self.prevMouseMode)
end

function ScreenTest3D:update(dt)
    self.text:setString('Current FPS: %i', Window.currentFPS())

    -- self.subMesh:rotate(0, 0, math.pi * dt / 2)
    self.subMesh:rotate(0, math.pi * dt / 2, 0)

    if self.camVelX ~= 0 or self.camVelY ~= 0 or self.camVelZ ~= 0 then
        local q = self.player:quaternion()
        self.player:move(q:apply(self.camVelX * dt, self.camVelY * dt, self.camVelZ * dt))
    end

    if self.lockOn then
        local Quaternion = require 'util.quaternion'
        local tx, ty, tz = self.cube:position(true)
        local px, py, pz = self.player:position(true)

        local x, y, z = tx - px, 0, tz - pz
        local len = x*x + z*z
        if len ~= 0 then
            len = math.sqrt(len)
            x, z = x/len, z/len
        end

        self.player:setRotation(Quaternion.fromToRotation(0, 0, -1, x, 0, z))

        x, y, z = 0, ty-py, -math.sqrt((tx-px)*(tx-px) + (tz-pz)*(tz-pz))
        len = y*y + z*z
        if len ~= 0 then
            len = math.sqrt(len)
            y, z = y/len, z/len
        end

        self.camera:setRotation(Quaternion.fromToRotation(0, 0, -1, x, y, z))
    end
end

function ScreenTest3D:render()
    Graphics.setDepthFunc('lequal')
        .enableDepthTest(true)
        .enableDepthMask(true)
        -- .setFillMode('wireframe')

    Graphics.setBlendMode('none')

    local width, height = Window.size()

    self:view():clear(0, 0, 0, 255)

    self.camera:setRenderbuffer(self.rb)
        :clear(200, 200, 200, 255)
        :setViewport(0, 0, width, height)
        :draw(self.scene)

    -- Graphics.setFillMode('solid')
    self.rb:texture('depth'):bind(1)
    self.depthShader:bind()
        :setSampler('uDepthBuf0', 1)
        :setUniform('uUnit', 0.75/width, 0.75/height)

    -- Graphics.enableDepthTest(false)
    --     .enableDepthMask(false)
    self:view():drawFsQuad(self.rb, nil, nil, self.depthShader)

    Graphics.setBlendMode('alpha')
        .setDepthFunc('lequal')
        .enableDepthTest(true)
        .enableDepthMask(true)

    self.camera:setRenderbuffer()
        :clear(0, 0, 0, 0, 1, false, false, false, false, true)
        :setViewport(0, 0, width, height)

    if require('system').platform('android', 'ios') then
        Graphics.enableColorWriteMask(true)
        self.camera:draw(self.scene)
        Graphics.enableColorWriteMask(false)
    end

    self.camera:draw(self.cube)
        
    self:view():draw(self.text)
end

function ScreenTest3D:keydown(scancode, keyCode, repeated)
    if repeated then return end

    if scancode == '1' then
        self:performTransition(Screen.back)
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
    elseif scancode == 'tab' then
        self.lockOn = not self.lockOn
    elseif scancode == '8' then
        self.rb:texture():setFilter 'bilinear'
    elseif scancode == '9' then
        self.rb:texture():setFilter 'nearest'
    end
end

function ScreenTest3D:keyup(scancode, keyCode)
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

function ScreenTest3D:mousemotion(x, y, xRel, yRel)
    self.player:rotate(0, -xRel * self.camSensitivity, 0)
    self.camera:rotate(-yRel * self.camSensitivity, 0, 0)
end

function ScreenTest3D:resized(width, height)
    self:view()
        :reset(0, 0, width, height)
        :setViewport(0, 0, width, height)

    local potW, potH = Util.pot(width, height)
    self.rb = RenderBuffer:new(potW, potH, true)
    self.rb:texture():setFilter('nearest')
    self.rb:texture('depth'):setFilter('nearest')

    self.camera:setRenderbuffer(self.rb)
        :setViewport(0, 0, width + 1, height + 1)
        :setPerspective(70, width/height, .1, 100)
end

function ScreenTest3D:updateParent()
    return self.parent and self.parent.class.name == 'screen.title'
end

return ScreenTest3D
