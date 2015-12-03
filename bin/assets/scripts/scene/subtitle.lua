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

------------------------------------------------------------
-- A test scene to be pushed over title
------------------------------------------------------------
local class = require 'nx.class'
local Scene = require 'scene'
local SceneSubtitle = class('scene.subtitle', Scene)

------------------------------------------------------------
local Mouse = require 'nx.mouse'
local Renderer = require 'nx.renderer'
local Thread = require 'nx.thread'
local Camera2D = require 'nx.camera2d'
local Shader = require 'nx.shader'
local Texture2D = require 'nx.texture2d'
local Renderbuffer = require 'nx.renderbuffer'
local Sprite = require 'nx.sprite'
local Font = require 'nx.vectorfont'
local Window = require 'nx.window'
local Text = require 'nx.text'
local Shape = require 'nx.shape'
local SoundSource = require 'nx.soundsource'
local MusicSource = require 'nx.musicsource'
local Cache = require 'game.cache'
local ffi = require 'ffi'
local C = ffi.C

local GameFont = require 'game.font'

------------------------------------------------------------
function SceneSubtitle:needsPreload()
    return true
end

------------------------------------------------------------
function SceneSubtitle:preload(worker)
    worker:addFile('nx.soundsource', 'assets/test.wav')
    worker:addFile('nx.image', 'assets/pasrien.png')

    self.musicSource = MusicSource:new()
    worker:addTask(function(music)
        music:open('assets/askepticshypothesis.ogg')
    end, self.musicSource)
end

------------------------------------------------------------
function SceneSubtitle:load()
    self.camera = Camera2D:new()

    self.echoFilter = require('nx.audioechofilter'):new()
    self.echoFilter:setParams(.5, .5)

    self.voiceGroup = require('nx.audiovoicegroup'):new()

    self.audiobus = require('nx.audiobus'):new()
    --self.audiobus:setFilter(self.echoFilter)
    self.audiobus:play()

    self.soundSource = Cache.get('assets/test.wav')
    self.soundSource:setLooping(true)
    self.voiceGroup:add(self.soundSource:playThrough(self.audiobus, -1, 0, true))

    self.musicSource:setVolume(.1)
    self.voiceGroup:add(self.musicSource:playThrough(self.audiobus, -1, 0, true))

    self.voiceGroup:pause(false)

    -- Create texture
    self.texture = Texture2D:new()
    self.texture:load(Cache.get('assets/pasrien.png'))
    self.texture:setFilter('nearest')
    self.texture:setAnisotropyLevel(8)
    self.texture:setRepeating('clamp', 'wrap')

    -- Create renderbuffer
    self.rb = Renderbuffer:new()
    if not self.rb:create(1024, 512) then
        print('Could not create render buffer')
    end
    self.rb:texture():setRepeating('wrap', 'wrap')

    self.sprite = Sprite:new(self.texture)
    self.sprite:setPosition(640, 360)
    self.sprite:setOrigin(256, 256)

    self.rbSprite = Sprite:new(self.rb:texture(), 0, 0, 640, 360)
    self.rbSprite:setPosition(640, 360)
    self.rbSprite:setOrigin(320, 180)

    self.fontSprite = Sprite:new(GameFont:texture(40))

    self.text = Text:new()
    self.text:setCharacterSize(40)
    self.text:setFont(GameFont)
    self.text:setString("Hello world 真島e ヒロ")

    self.shape = Shape:new()
    self.shape:setTexture(self.texture)
    self.shape:setVertexData('trianglestrip', true,
        200, 200, 255, 0, 0, 255, 0, 0,
        100, 300, 0, 255, 0, 255, 0, 1,
        300, 300, 0, 0, 255, 255, 1, 0,
        300, 300, 0, 0, 255, 255, 1, 0,
        100, 300, 0, 255, 0, 255, 0, 1,
        300, 300, 0, 0, 255, 255, 1, 0,
        200, 400, 255, 255, 255, 255, 1, 1
    )
    self.shape:setIndexData(0, 1, 3, 6)

    self.touchTexts = {}
    self._processParent = true

    self:performTransition(self.camera)
end

------------------------------------------------------------
function SceneSubtitle:update(dt)
    self.sprite:setRotation(math.pi * dt + self.sprite:rotation())

    local w, h = Window.size()
    local fps = Window.currentFPS()
    self.text:setString(('FPS: %i\nScreen size: %i %i'):format(fps, w, h))
end

------------------------------------------------------------
function SceneSubtitle:render()
    self.camera:setRenderbuffer(self.rb)
    self.camera:setViewport(0, 0, 640, 360)
    self.camera:clear(255, 255, 255, 128)
    self.camera:draw(self.sprite)

    self.camera:setRenderbuffer(nil)
    self.camera:setViewport(nil)
    self.camera:draw(self.rbSprite)

    self.camera:draw(self.text)
    -- self.camera:draw(self.fontSprite)

    self.camera:draw(self.shape)

    for i, text in pairs(self.touchTexts) do
        self.camera:draw(text)
    end

    -- Renderer.drawFsQuad(self.rb:texture(), 640, 360)
end

------------------------------------------------------------
function SceneSubtitle:onKeyDown(scancode)
    if scancode == 'f2' then
        self:callNewScene()
        return false
    elseif scancode == 'f3' then
        self._processParent = not self._processParent
    elseif scancode == 'space' then
        Thread:new(function(texture)
            print('a')
            require('nx.window').ensureContext()
            print('b')
            local img = require('nx.image').create(1024, 1024, 255, 128, 0, 255)
            img:copy(texture:data(), 0, 0, 1024, 0, 256, 1024, 512, true)
            img:save('helloworld.png')
            print('c')
        end, self.rb:texture()):detach()
    end
end

------------------------------------------------------------
function SceneSubtitle:onTouchDown(finger, x, y)
    local w, h = self.camera:size()
    x, y = x * w, y * h

    local text = Text:new()
    text:setString("Finger: " .. tostring(finger))
    text:setFont(GameFont)
    text:setPosition(x, y)
    text:setColor(0, 0, 0)

    local bX, bY, bW, bH = text:bounds()
    text:setOrigin(bW / 2, 150)

    self.touchTexts[finger] = text

    if finger == 2 then
        self:callNewScene()
    elseif finger == 3 then
        self:backToScene()
    end

    return false
end

------------------------------------------------------------
function SceneSubtitle:onTouchUp(finger, x, y)
    self.touchTexts[finger] = nil
end

------------------------------------------------------------
function SceneSubtitle:onTouchMotion(finger, x, y)
    if self.touchTexts[finger] then
        local w, h = self.camera:size()
        x, y = x * w, y * h
        self.touchTexts[finger]:setPosition(x, y)
    end
end

------------------------------------------------------------
function SceneSubtitle:processParent()
    return self._processParent
end

------------------------------------------------------------
function SceneSubtitle:back(scene, msg)
    self:performTransition(self.camera)
    print(scene, msg)
end

------------------------------------------------------------
function SceneSubtitle:release()
    self.texture:release()
    self.rb:release()
    self.sprite:release()
    self.rbSprite:release()

    self.audiobus:stop()
    Cache.release('assets/test.wav')
    Cache.release('assets/pasrien.png')
end

------------------------------------------------------------
function SceneSubtitle:callNewScene()
    Scene.push('scene.subtitle')
end

------------------------------------------------------------
function SceneSubtitle:backToScene()
    self:performTransition(self.camera, function()
        Scene.back('hello from SceneSubtitle')
    end)
end

------------------------------------------------------------
return SceneSubtitle
