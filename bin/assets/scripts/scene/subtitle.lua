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
local Matrix4 = require 'nx.matrix4'
local Mouse = require 'nx.mouse'
local Renderer = require 'nx.renderer'
local Thread = require 'nx.thread'
local Camera2D = require 'nx.camera2d'
local Shader = require 'nx.shader'
local Texture = require 'nx.texture'
local Renderbuffer = require 'nx.renderbuffer'
local Sprite = require 'nx.sprite'
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        float a, b, c, d;
    } nxVertex;
]]

------------------------------------------------------------
function SceneSubtitle:load()
    self:releaseTest()

    self.camera = Camera2D:new(0, 0, 1280, 720)

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

    -- Create renderbuffer
    self.rb = Renderbuffer:new()
    if not self.rb:create(1280, 720) then
        print('Could not create render buffer')
    end

    self.rb:texture():setRepeating('wrap', 'wrap')

    self.sprite = Sprite:new(self.texture)
    self.rbSprite = Sprite:new(self.rb:texture())

    self.initialized = true
    self._processParent = true
end

------------------------------------------------------------
function SceneSubtitle:release()
    self:releaseTest()
end

------------------------------------------------------------
function SceneSubtitle:render()
    C.nxRendererSetCullMode(3)
    C.nxRendererSetBlendMode(true, 2, 3)

    Renderer.setRenderbuffer(self.rb)
    Renderer.setViewport(0, 0, 1024, 1024)
    Renderer.clear(255, 255, 255, 128)

    -- self.texture:bind(1)

    -- self.defShader:bind()
    -- self.defShader:setSampler('uTexture', 1)

    -- C.nxRendererDraw(1, 0, 4)

    self.sprite:_render(nil, self.camera:matrix())

    Renderer.setRenderbuffer(nil)
    Renderer.setViewport(0, 0, 1280, 720)

    -- self.rb:texture():bind(2)

    -- self.rbShader:bind()
    -- self.rbShader:setUniform('uViewMat', self.camera:matrix())
    -- self.rbShader:setSampler('uTexture', 2)

    -- C.nxRendererDraw(1, 0, 4)

    self.rbSprite:_render(nil, self.camera:matrix())
end

------------------------------------------------------------
function SceneSubtitle:onKeyDown(scancode)
    if scancode == 'f2' then
        Scene.back()
        return false
    elseif scancode == 'f3' then
        self._processParent = not self._processParent
    elseif scancode == 'space' then
        local img = require('nx.image').create(1024, 1024, self.rb:texture():data())
        img:save('helloworld.png')
    end
end

------------------------------------------------------------
function SceneSubtitle:processParent()
    return self._processParent
end

------------------------------------------------------------
function SceneSubtitle:releaseTest()
    if not self.initialized then return end

    self.rbShader:release()
    self.defShader:release()
    self.texture:release()
    self.rb:release()
    C.nxRendererDestroyBuffer(self.vbRect)

    self.initialized = false
end

------------------------------------------------------------
return SceneSubtitle