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
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        float x, y;
        float u, v;
    } NxShapeVertexPosCoords;

    typedef struct {
        float x, y;
        uint8_t r, g, b, a;
        float u, v;
    } NxShapeVertexPosColorCoords;
]]

------------------------------------------------------------
-- Represents a 2D shape
------------------------------------------------------------
local Entity2D = require 'nx.entity2d'
local Shape = Entity2D:subclass('nx.shape')

local Renderer = require 'nx.renderer'
local Arraybuffer = require 'nx.arraybuffer'

------------------------------------------------------------
local defaultTexture = require('nx.texture'):new()
defaultTexture:create('2d', 1, 1, 1, 0, 0)
defaultTexture:setData(ffi.new('uint8_t[4]', {255, 255, 255, 255}), 0, 0)

local toPrimitive = {
    triangles = 0,
    trianglestrip = 1,
    lines = 2 -- TODO
}

------------------------------------------------------------
local function vertexStruct(hasColor)
    if hasColor then
        return 'NxShapeVertexPosColorCoords', 8, 20
    else
        return 'NxShapeVertexPosCoords', 4, 16
    end
end

------------------------------------------------------------
function Shape.static._defaultShader(hasColor)
    return Renderer.defaultShader(hasColor and 2 or 1)
end

------------------------------------------------------------
function Shape.static._vertexLayout(hasColor)
    return Renderer.vertexLayout(hasColor and 2 or 1)
end

------------------------------------------------------------
function Shape:initialize()
    Entity2D.initialize(self)
end

------------------------------------------------------------
function Shape:setTexture(texture)
    self._texture = texture
end

------------------------------------------------------------
function Shape:texture()
    return self._texture
end

------------------------------------------------------------
function Shape:setVertexData(primitive, hasColor, a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) ~= 'table' then return end

    self._primitive = toPrimitive[primitive] or 0
    self._hasColor = hasColor
    local structName, valueCount, vertexSize = vertexStruct(hasColor)
    self._vertexSize = vertexSize

    local buffer
    if type(a[0]) == 'table' then
        self._vertexCount = #a
        buffer = ffi.new(structName .. '[?]', self._vertexCount, a)
    else
        self._vertexCount = #a / valueCount
        buffer = ffi.new(structName .. '[?]', self._vertexCount)
        for i = 1, self._vertexCount do
            local vertex = {}
            for j = 1, valueCount do
                vertex[j] = a[(i-1)*valueCount + j]
            end
            buffer[i - 1] = ffi.new(structName, vertex)
        end
    end

    self._vertexBuffer = Arraybuffer.vertexbuffer(ffi.sizeof(buffer), buffer)
end

------------------------------------------------------------
function Shape:setIndexData(a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) ~= 'table' then return end

    local buffer = ffi.new('uint16_t[?]', #a, a)

    self._indexCount = #a
    self._indexBuffer = Arraybuffer.indexbuffer(ffi.sizeof(buffer), buffer)
end

------------------------------------------------------------
function Shape:_render(camera, state)
    if self._vertexBuffer then
        local texture = self._texture or defaultTexture
        texture:bind(0)

        local shader = self._shader or Shape._defaultShader(self._hasColor)
        shader:bind()
        shader:setUniform('uTransMat', state:matrix())
        shader:setUniform('uColor', state:color(true))
        shader:setUniform('uTexSize', 1, 1)
        shader:setSampler('uTexture', 0)

        Arraybuffer.setVertexbuffer(self._vertexBuffer, 0, 0, self._vertexSize)
        Arraybuffer.setIndexbuffer(self._indexBuffer, 16)
        C.nxRendererSetVertexLayout(Shape._vertexLayout(self._hasColor))

        if self._indexBuffer then
            C.nxRendererDrawIndexed(self._primitive, 0, self._indexCount)
        else
            C.nxRendererDraw(self._primitive, 0, self._vertexCount)
        end
    end
end

------------------------------------------------------------
return Shape