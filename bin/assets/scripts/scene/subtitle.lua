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
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        float a, b, c, d;
        uint8_t e, f, g, h;
    } nxVertex;
]]

------------------------------------------------------------
function SceneSubtitle:load()
    self.camera = Camera2D:new(-1, -1, 2, 2)

    -- self.camera:zoom(0.5, 0.5)

    self:releaseTest()

    -- Register vertex layouts
    local attribs = ffi.new('NxVertexLayoutAttrib[?]', 3, {
        {'vertPos',    0, 2, 0,  0},
        {'texCoords0', 0, 2, 8,  0},
        {'col0',       0, 4, 16, 1}
    })
    self.vlShape = C.nxRendererRegisterVertexLayout(3, attribs)

    -- Create triangle
    local verts = ffi.new('nxVertex[?]', 4, {
        {-0.5, -0.5, 0, 0, 255, 255, 255, 255},
        { 0.5, -0.5, 1, 0, 255, 0,   0,   255},
        {-0.5,  0.5, 0, 1, 0,   255, 0,   255},
        { 0.5,  0.5, 1, 1, 0,   0,   255, 255}
    })
    self.vbRect = C.nxRendererCreateVertexBuffer(ffi.sizeof(verts), verts)

    -- Create shaders
    self.defShader = Shader:new()
    local ok, err = self.defShader:load([[
        attribute vec2 vertPos;
        attribute vec2 texCoords0;
        attribute vec4 col0;
        varying vec4 color;
        varying vec2 coords;
        void main() {
            color = col0;
           coords = texCoords0;
           gl_Position = vec4(vertPos, 0.0, 1.0);
        }]], [[
        uniform sampler2D tex;
        varying vec4 color;
        varying vec2 coords;
        void main() {
           vec4 col = texture2D(tex, coords);
           gl_FragColor = col * vec4(1.0, 1.0, 1.0, 1.0) * color;
        }]]
    )
    if not ok then print(err) end

    self.rbShader = Shader:new()
    ok, err = self.rbShader:load([[
        attribute vec2 vertPos;
        attribute vec2 texCoords0;
        attribute vec4 col0;
        uniform mat4 projectionMat;
        varying vec2 coords;
        void main() {
           coords = texCoords0;
           gl_Position = projectionMat * vec4(vertPos, 0.0, 1.0);
        }]], [[
        uniform sampler2D tex2;
        varying vec2 coords;
        void main() {
           // gl_FragColor = vec4(1.0, 1.0, 0.0, 0.5);
           gl_FragColor = texture2D(tex2, coords);
        }]]
    )
    if not ok then print(err) end

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
    self.texture:setAnisotropyLevel(16)
    self.texture:setRepeating('clamp', 'wrap')

    -- Create renderbuffer
    self.rb = Renderbuffer:new()
    self.rb:create(1024, 1024)
    if not self.rb:create(1024, 1024) then
        print('Could not create render buffer')
    end

    self.rb:texture():setRepeating('wrap', 'wrap')

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

    C.nxRendererSetVertexBuffer(0, self.vbRect, 0, 20)
    C.nxRendererSetIndexBuffer(0, 0)
    C.nxRendererSetVertexLayout(self.vlShape)

    Renderer.setRenderbuffer(self.rb)
    Renderer.setViewport(0, 0, 1024, 1024)
    Renderer.clear(255, 255, 0, 50)

    self.texture:bind(1)

    self.defShader:bind()
    self.defShader:setSampler('tex', 1)

    C.nxRendererDraw(1, 0, 4)

    Renderer.setRenderbuffer(nil)
    Renderer.setViewport(0, 0, 1280, 720)

    self.rb:texture():bind(2)

    C.nxRendererSetBlendMode(true, 2, 3)

    self.rbShader:bind()
    self.rbShader:setUniform('projectionMat', self.camera:matrix())
    self.rbShader:setSampler('tex2', 2)

    C.nxRendererDraw(1, 0, 4)
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