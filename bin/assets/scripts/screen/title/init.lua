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

local Log       = require 'util.log'
local Graphics  = require 'graphics'
local Window    = require 'window'
local Screen    = require 'screen'
local GameFont  = require 'game.font'
local GameState = require 'game.state'

local ScreenTitle = Screen:subclass('screen.title')

function ScreenTitle:initialize(firstRun)
    if firstRun then
        local caps = Graphics.getCapabilities()

        Log.info('GPU Capabilities:')
        for i, v in pairs(caps) do
            Log.info('%s: %s', i, v)
        end

        self:setPreloadParams({
            message = 'INITIALIZING %i%%'
        })
    end

    self.logoTexture = self:cache('tex2d:assets/textures/logo.png')

    -- Check for saved games
    local states = GameState.list()
    if #states > 0 then
        GameState.current = self:cache('state:' .. states[1])
    end
end

function ScreenTitle:entered()
    self.fps = require('graphics.text')
        :new('', GameFont, 14)
        :setPosition(10, 10)

    local logoW, logoH = self.logoTexture:size()
    self.logoTexture:setFilter('nearest')

    self.logo = require('graphics.sprite')
        :new(self.logoTexture)
        :setOrigin(logoW/2, logoH/3)
end

function ScreenTitle:update(dt)
    self.fps:setString('Current FPS: %i', Window.currentFPS())
end

function ScreenTitle:render()
    self:view()
        :clear()
        :draw(self.fps)
        :draw(self.logo)

    if self ~= Screen.currentScreen() then
        self:view():fillFsQuad(0, 0, 0, 230)
    end
end

function ScreenTitle:resized(width, height)
    Screen.resized(self, width, height)

    self.logo:setPosition(width/2, height/3)
    if width >= 1920 and height >= 1080 then
        self.logo:setScaling(math.floor(width/960), math.floor(height/540))
    else
        self.logo:setScaling(1, 1)
    end
end

function ScreenTitle:openMenu()
    Screen.push('screen.title.menu')
end

function ScreenTitle:mousedown(x, y, button)
    self:openMenu()
end

function ScreenTitle:keydown(scancode, keyCode, repeated)
    if scancode == 'f10' then
        Window.create('m2n-', 1280, 720, {})
    elseif scancode == 'f11' then
        Window.create('m2n-', 1920, 1080, {fullscreen = true})
    elseif scancode == 'f12' then
        Window.create('m2n-', 1024, 720, {fullscreen = true})
    elseif scancode == '2' then
        self:performTransition(Screen.push, 'screen.test.3d')
    elseif scancode == '\\' then
        self:performTransition(Screen.goTo, 'screen.title')
    elseif scancode == 'p' then
        self:performTransition(Screen.push, 'screen.title')
        return false
    else
        self:openMenu()
    end
end

function ScreenTitle:touchdown(id, posX, posY)
    if id == 2 then
        self:openMenu()
    end
end

return ScreenTitle
