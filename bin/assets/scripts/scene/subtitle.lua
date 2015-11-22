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
local Texture = require 'nx.texture'
local Renderbuffer = require 'nx.renderbuffer'
local Sprite = require 'nx.sprite'
local Font = require 'nx.vectorfont'
local Window = require 'nx.window'
local Text = require 'nx.text'
local Shape = require 'nx.shape'
local SoundSource = require 'nx.soundsource'
local MusicSource = require 'nx.musicsource'
local ffi = require 'ffi'
local C = ffi.C

------------------------------------------------------------
function SceneSubtitle:load()
    self.camera = Camera2D:new(0, 0, 1280, 720)

    self.echoFilter = require('nx.audioechofilter'):new()
    self.echoFilter:setParams(.5, .5)
    require('nx.audio').setGlobalFilter(self.echoFilter)

    self.voiceGroup = require('nx.audiovoicegroup'):new()

    self.soundSource = SoundSource:new()
    self.soundSource:open('assets/test.wav')
    self.soundSource:setLooping(true)
    self.soundSource:set3dListenerRelative(false)
    self.voiceGroup:add(self.soundSource:play(-1, 0, true))

    self.musicSource = MusicSource:new()
    self.musicSource:open('assets/askepticshypothesis.ogg')
    self.musicSource:setVolume(.1)
    self.voiceGroup:add(self.musicSource:play(-1, 0, true))

    self.voiceGroup:pause(false)

    -- Load image
    local img = require('nx.image'):new()
    local ok, err = img:load('assets/pasrien.png')
    if not ok then
        print("Couldn't load image: " .. err)
    end
    local imgWidth, imgHeight = img:size()

    -- Create texture
    self.texture = Texture:new()
    self.texture:create('2d', imgWidth, imgHeight)
    self.texture:setData(img)
    self.texture:setFilter('nearest')
    self.texture:setAnisotropyLevel(8)
    self.texture:setRepeating('clamp', 'wrap')

    img:release()

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

    self.font = Font:new()
    self.font:open('assets/mplus-1c-regular.ttf')
    for i = 48, 126 do
        self.font:glyph(i, 30)
    end

    self.fontSprite = Sprite:new(self.font:texture(30))

    self.text = Text:new()
    self.text:setCharacterSize(14)
    self.text:setFont(self.font)
    self.text:setString("Hello world 真島 ヒロ")

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

    self._processParent = true
end

------------------------------------------------------------
function SceneSubtitle:update(dt)
    local w, h = Window:size()
    self.text:setString('FPS: ' .. math.floor(1/dt + .5)
        .. '\nScreen size: ' .. w .. ' ' .. h)
    self.sprite:setRotation(math.pi * dt + self.sprite:rotation())
end

------------------------------------------------------------
function SceneSubtitle:render()
    C.nxRendererSetCullMode(3)
    C.nxRendererSetBlendMode(true, 2, 3)

    self.camera:setRenderbuffer(self.rb)
    self.camera:setViewport(0, 0, 640, 360)
    self.camera:clear(255, 255, 255, 128)
    self.camera:draw(self.sprite)

    self.camera:setRenderbuffer(nil)
    self.camera:setViewport(nil, nil, nil, nil)
    self.camera:draw(self.rbSprite)

    --self.camera:draw(self.fontSprite)
    self.camera:draw(self.text)

    self.camera:draw(self.shape)

    -- Renderer.drawFsQuad(self.rb:texture(), 640, 360)
end

------------------------------------------------------------
function SceneSubtitle:onKeyDown(scancode)
    if scancode == 'f2' then
        Scene.back('hello from SceneSubtitle')
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
function SceneSubtitle:processParent()
    return self._processParent
end

------------------------------------------------------------
function SceneSubtitle:release()
    self.texture:release()
    self.rb:release()
    self.sprite:release()
    self.rbSprite:release()
end

------------------------------------------------------------
return SceneSubtitle