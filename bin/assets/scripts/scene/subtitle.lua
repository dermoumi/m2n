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
local ffi = require 'ffi'
local C = ffi.C

------------------------------------------------------------
function SceneSubtitle:load()
    self:releaseTest()

    -- Register vertex layouts
    local attribs = ffi.new('NxVertexLayoutAttrib[?]', 2, {
        ffi.new('NxVertexLayoutAttrib', {
            'vertPos', 0, 2, 0        
        }),
        ffi.new('NxVertexLayoutAttrib', {
            'texCoords0', 0, 2, 8
        })
    })
    self.vlShape = C.nxRendererRegisterVertexLayout(2, attribs)

    -- Create triangle
    local verts = ffi.new('float[?]', 4*4, {
        -0.5, -0.5, 0, 0,
         0.5, -0.5, 1, 0,
        -0.5,  0.5, 0, 1,
         0.5,  0.5, 1, 1
    })
    self.vbRect = C.nxRendererCreateVertexBuffer(ffi.sizeof(verts), verts)

    -- Create shaders
    self.defShader = C.nxRendererCreateShader([[
        attribute vec2 vertPos;
        attribute vec2 texCoords0;
        varying vec2 coords;
        void main() {
           coords = texCoords0;
           gl_Position = vec4(vertPos, 0.0, 1.0);
        }]], [[
        uniform sampler2D tex;
        varying vec2 coords;
        void main() {
           vec4 col = texture2D(tex, coords);
           gl_FragColor = col * vec4(1.0, 1.0, 0.5, 1.0);
        }]]
    )
    if self.defShader == 0 then
        print(ffi.string(nxRendererGetShaderLog()))
    end

    self.rbShader = C.nxRendererCreateShader([[
        attribute vec2 vertPos;
        attribute vec2 texCoords0;
        varying vec2 coords;
        void main() {
           coords = texCoords0;
           gl_Position = vec4(vertPos, 0.0, 1.0);
        }]], [[
        uniform sampler2D tex2;
        varying vec2 coords;
        void main() {
           // gl_FragColor = vec4(1.0, 1.0, 0.0, 0.5);
           gl_FragColor = texture2D(tex2, coords);
        }]]
    )
    if self.rbShader == 0 then
        print(ffi.string(C.nxRendererGetShaderLog()))
    end
    
    -- Load image
    local img, err = require('nx.image').load('assets/pasrien.png')
    if not img then
        print("Couldn't load image: " .. err)
    end
    local imgWidth, imgHeight = img:size()

    -- Create texture
    self.texture = C.nxRendererCreateTexture(0, imgWidth, imgHeight, 1, 1, true, true, false)
    if self.texture == 0 then print('Could not create texture') end
    C.nxRendererUploadTextureData(self.texture, 0, 0, img:data())

    -- Create renderbuffer
    self.rb = C.nxRendererCreateRenderbuffer(1024, 1024, 1, false, 1, 0)
    if self.rb == 0 then print('Could not create render buffer') end
    self.rbTex = C.nxRendererGetRenderbufferTexture(self.rb, 0)

    -- Reset states
    C.nxRendererResetStates()

    self.initialized = true

    self._processParent = true
end

------------------------------------------------------------
function SceneSubtitle:release()
    self:releaseTest()
    print('bye?')
end

------------------------------------------------------------
function SceneSubtitle:render()
    C.nxRendererSetRenderbuffer(self.rb)

    C.nxRendererSetViewport(0, 0, 1024, 1024)
    C.nxRendererClear(255, 255, 255, 50, 0, true, false, false, false, false)

    C.nxRendererSetVertexBuffer(0, self.vbRect, 0, 16)
    C.nxRendererSetIndexBuffer(0, 0)
    C.nxRendererSetVertexLayout(self.vlShape)

    C.nxRendererSetTexture(1, self.texture, 0)

    C.nxRendererBindShader(self.defShader)

    local loc = C.nxRendererGetShaderSamplerLoc(self.defShader, 'tex')
    if loc >= 0 then C.nxRendererSetShaderSampler(loc, 1) end

    C.nxRendererDraw(1, 0, 4)

    C.nxRendererSetRenderbuffer(0)
    C.nxRendererSetViewport(0, 0, 1280, 720)

    C.nxRendererSetTexture(2, self.rbTex, 0)

    C.nxRendererBindShader(self.rbShader)
    C.nxRendererSetBlendMode(true, 2, 3)

    loc = C.nxRendererGetShaderSamplerLoc(self.rbShader, 'tex2')
    if loc >= 0 then C.nxRendererSetShaderSampler(loc, 2) end

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
        local buffer = ffi.new('uint8_t[?]', 1024 * 1024 * 8)
        C.nxRendererGetTextureData(self.rbTex, 0, 0, buffer)
        local img = require('nx.image').create(1024, 1024, buffer)
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

    C.nxRendererDestroyShader(self.defShader)
    C.nxRendererDestroyShader(self.rbShader)
    C.nxRendererDestroyBuffer(self.vbRect)
    C.nxRendererDestroyTexture(self.texture)
    C.nxRendererDestroyRenderbuffer(self.rb)

    self.initialized = false
end

------------------------------------------------------------
return SceneSubtitle