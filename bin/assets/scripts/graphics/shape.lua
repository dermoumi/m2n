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

local Graphics     = require 'graphics'
local VertexBuffer = require 'graphics.vertexbuffer'
local IndexBuffer  = require 'graphics.indexbuffer'
local Entity2D     = require 'graphics.entity2d'

local Shape = Entity2D:subclass 'graphics.shape'

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

local toPrimitive = {
    points = 0,
    lines = 1,
    linestrip = 2,
    lineloop = 3,
    triangles = 4,
    trianglestrip = 5,
    trianglefan = 6
}

local function vertexStruct(hasColor)
    if hasColor then
        return 'NxShapeVertexPosColorCoords', 8, 20
    else
        return 'NxShapeVertexPosCoords', 4, 16
    end
end

function Shape.static._defaultShader(hasColor)
    return Graphics.defaultShader(hasColor and 2 or 1)
end

function Shape.static._vertexLayout(hasColor)
    return Graphics.vertexLayout(hasColor and 2 or 1)
end

function Shape:initialize()
    Entity2D.initialize(self)
end

function Shape:setTexture(texture)
    self._texture = texture

    return self
end

function Shape:setVertexData(primitive, hasColor, a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) == 'table' then
        self._primitive  = toPrimitive[primitive] or 0
        self._hasColor   = hasColor

        local structName, valueCount, vertexSize = vertexStruct(hasColor)

        local buffer
        if type(a[0]) == 'table' then
            local vertexCount = #a
            buffer = ffi.new(structName .. '[?]', vertexCount, a)
        else
            local vertexCount = #a / valueCount
            buffer = ffi.new(structName .. '[?]', vertexCount)
            for i = 1, vertexCount do
                local vertex = {}
                for j = 1, valueCount do
                    vertex[j] = a[(i-1)*valueCount + j]
                end
                buffer[i-1] = ffi.new(structName, vertex)
            end
        end

        self._vertexBuffer = VertexBuffer:new(buffer, ffi.sizeof(buffer), vertexSize)
    else
        self._vertexBuffer = nil
    end

    return self
end

function Shape:setIndexData(a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) == 'table' then
        local buffer = ffi.new('uint16_t[?]', #a, a)

        self._indexBuffer = IndexBuffer:new(buffer, ffi.sizeof(buffer), '16')
    else
        self._indexBuffer = nil
    end

    return self
end

function Shape:texture()
    return self._texture
end

function Shape:_render(camera)
    if self._vertexBuffer then
        local texture = self._texture or Graphics.defaultTexture()
        texture:bind(0)

        local shader = self._shader or Shape._defaultShader(self._hasColor)
        shader:bind()
        shader:setUniform('uProjMat', camera:projection())
        shader:setUniform('uTransMat', self:matrix(true))
        shader:setUniform('uColor', self:color(true, true))
        shader:setUniform('uTexSize', 1, 1)
        shader:setSampler('uTexture0', 0)

        VertexBuffer.bind(self._vertexBuffer)
        IndexBuffer.bind(self._indexBuffer)
        C.nxRendererSetVertexLayout(Shape._vertexLayout(self._hasColor))

        if self._indexBuffer then
            C.nxRendererDrawIndexed(self._primitive, 0, self._indexBuffer:count())
        else
            C.nxRendererDraw(self._primitive, 0, self._vertexBuffer:count())
        end
    end
end

return Shape
