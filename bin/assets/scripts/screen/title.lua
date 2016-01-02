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

local Log      = require 'util.log'
local Mouse    = require 'window.mouse'
local Graphics = require 'graphics'
local Window   = require 'window'
local Screen   = require 'screen'

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

    self:worker():addFile('graphics.image', 'assets/pasrien.png')
    self:worker():addFile('graphics.image', 'assets/cursor.png')
    self:worker():addFile('audio.source', 'assets/test.wav')

    self.musicSource = require('audio.source'):new()
    self:worker():addTask(function(music)
        music:open('assets/undersodiumbulb.ogg')
    end, self.musicSource)
end

function ScreenTitle:load()
    self.text = require('graphics.text')
        :new('', require 'game.font', 14)
        :setPosition(10, 10)

    self.sprite = require('graphics.sprite')
        :new(self:cache('assets/pasrien.png'))
        :setPosition(100, 100)
        -- :addChild(self.text)

    self.voiceGroup = require('audio.voicegroup'):new()
    self.echoFilter = require('audio.echofilter'):new()
        :setParams(.5, .5)

    self.voiceGroup = require('audio.voicegroup'):new()

    self.audiobus = require('audio.bus'):new()
        -- :setFilter(self.echoFilter)
    self.audiobus:play()

    self.soundSource = self:cache('assets/test.wav')
        :setLooping(true)
    self.voiceGroup:add(self.soundSource:playThrough(self.audiobus, -1, 0, true))

    self.musicSource:setVolume(.1)
    self.voiceGroup:add(self.musicSource:playThrough(self.audiobus, -1, 0, true))

    self.voiceGroup:pause(false)

    Mouse.setCursor(self:cache('assets/cursor.png'), 4, 4)
end

function ScreenTitle:update(dt)
    self.text:setString('Current FPS: %i', Window.currentFPS())
end

function ScreenTitle:render()
    self:view():clear()

    self:view():draw(self.text)
        :draw(self.sprite:setPosition(100, 100))
        :draw(self.sprite:setPosition(600, 100))
end

function ScreenTitle:onKeyDown(scancode, keyCode, repeated)
    if scancode == '2' then
        self:performTransition(Screen.push, 'screen.test.3d')
    elseif scancode == 'f10' then
        Window.create('m2n-', 1280, 720, {})
    elseif scancode == 'f11' then
        Window.create('m2n-', 1920, 1080, {fullscreen = true, msaa = 4})
    elseif scancode == 'f12' then
        Window.create('m2n-', 1024, 720, {fullscreen = true, msaa = 8})
    elseif scancode == 'f9' then
        if Keyboard.modKeyDown('ctrl') then
            collectgarbage('collect')
        else
            print('Lua usage: ' .. tostring(collectgarbage('count') * 1024))
        end
    elseif scancode == '\\' then
        self:performTransition(Screen.goTo, 'screen.title')
    elseif scancode == 'return' then
        print(Window.showMessageBox('test', 'helloworld!', {
            'ok',
            'cancel'
        }))
    end
end

function ScreenTitle:release()
    self.audiobus:stop()
end

return ScreenTitle
