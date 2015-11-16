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
-- Represents a 2D sprite in a 2D space
------------------------------------------------------------
local class = require 'nx.class'
local Entity2D = require 'nx.entity2d'
local Sprite = class('nx.sprite', Entity2D)

local Renderer = require 'nx.renderer'
local ffi = require 'ffi'
local C = ffi.C

------------------------------------------------------------
function Sprite.static._defaultShader()
    return Renderer.defaultShader(1)
end

------------------------------------------------------------
function Sprite.static._vertexLayout()
    return Renderer.vertexLayout(1)
end

------------------------------------------------------------
function Sprite:initialize(texture, subX, subY, subW, subH, normalized)
    Entity2D.initialize(self)

    if not subX then
        self:setTexture(texture)
    else
        self:setTexture(texture, true)
        self:setSubrect(subX, subY, subW, subH, normalized)
    end
end

------------------------------------------------------------
function Sprite:release()
    if not self._vertexbuffer then return end
    C.nxRendererDestroyBuffer(ffi.gc(self._vertexbuffer, nil))
end

------------------------------------------------------------
function Sprite:setTexture(texture, keepSubrect)
    if texture:texType() ~= '2d' then -- only accept 2D textures
        return
    end

    self._texture = texture

    if keepSubrect then
        self._updateBuffer = true
    else
        local w, h = texture:size()
        self:setSubrect(0, 0, w, h)
    end
end

------------------------------------------------------------
function Sprite:setSubrect(subX, subY, subW, subH, normalized)
    self._subX = subX or 0
    self._subY = subY or 0
    self._subW = subW or 0
    self._subH = subH or 0

    self._normalized = normalized

    self._updateBuffer = true
end

------------------------------------------------------------
function Sprite:texture()
    return self._texture
end

------------------------------------------------------------
function Sprite:subrect()
    return self._subX, self._subY, self._subW, self._subH, self._normalized
end

------------------------------------------------------------
function Sprite:size()
    if not self._texture then return 0, 0 end
    if self._normalized then
        local w, h = self._texture:size(true)
        return self._subW * w, self._subH * h
    else
        return self._subW, self._subH
    end
end

------------------------------------------------------------
function Sprite:_render(camera, transMat, r, g, b, a)
    if not self._texture then return end

    if self._updateBuffer then
        local texW, texH = self._texture:size(true)
        local w, h, subL, subT, subR, subB

        -- Calculate normalized coordinates
        if self._normalized then
            w = texW * self._subW
            h = texH * self._subH

            subL = self._subX
            subT = self._subY
            subR = self._subX + self._subW
            subB = self._subY + self._subH
            print(texW, texH, self._subW, self._subH, self._subW * texW)
        else
            w = self._subW
            h = self._subH

            subL = self._subX / texW
            subT = self._subY / texH
            subR = (self._subX + self._subW) / texW
            subB = (self._subY + self._subH) / texH
        end

        if self._texture:flipCoords() then
            local temp = subT
            subT = subB
            subB = temp
        end

        local buffer = ffi.new('float[16]', {
            0, 0, subL, subT,
            w, 0, subR, subT,
            0, h, subL, subB,
            w, h, subR, subB
        })

        if not self._vertexbuffer then
            local handle = C.nxRendererCreateVertexBuffer(ffi.sizeof(buffer), buffer)
            self._vertexbuffer = ffi.gc(handle, C.nxRendererDestroyBuffer)
        else
            C.nxRendererUpdateBufferData(self._vertexbuffer, 0, ffi.sizeof(buffer), buffer)
        end

        self._updateBuffer = false
    end

    transMat:combine(self:matrix())

    if self._vertexbuffer ~= 0 then
        local shader = self._shader or Sprite._defaultShader()
        r = r or 255
        g = g or 255
        b = b or 255
        a = a or 255

        self._texture:bind(0)

        shader:bind()
        shader:setUniform('uTransMat', transMat)
        shader:setUniform('uColor', r / 255, g / 255, b / 255, a / 255)
        shader:setSampler('uTexture', 0)

        C.nxRendererSetVertexBuffer(0, self._vertexbuffer, 0, 16)
        C.nxRendererSetIndexBuffer(nil, 0)
        C.nxRendererSetVertexLayout(Sprite._vertexLayout())

        C.nxRendererDraw(1, 0, 4)
    end
end

------------------------------------------------------------
return Sprite