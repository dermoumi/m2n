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

local Graphics    = require 'graphics'
local Arraybuffer = require 'graphics.arraybuffer'
local class       = require 'class'

local Geometry = class 'graphics.geometry'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        float x, y, z, u, v;
    } NxMeshVertexPosCoords;
]]

local vertexSize, vertexLayout =
    ffi.sizeof('NxMeshVertexPosCoords'),
    Graphics.vertexLayout(3)

function Geometry.static.factory(task)
    task:addTask(function(geom, filename)
            local success = true

            local file = require('filesystem.inputfile'):new(filename)
                :onError(function() success = false end)

            local vertBuffer, vertBufSize, indexBuffer, indexBufSize

            vertBufSize, indexBufSize = file:readU32(), file:readU32()
            if vertBufSize > 0 then vertBuffer = file:read(vertBufSize) end
            if indexBufSize > 0 then indexBuffer = file:read(indexBufSize) end

            if not success then return false end

            geom:setVertexData(vertBuffer, vertBufSize)
                :setIndexData(indexBuffer, indexBufSize)
        end)
end

function Geometry.static.vertexDataToBuffer(a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) ~= 'table' then return ffi.new('NxMeshVertexPosCoords*'), 0, 0 end

    local buffer, vertCount
    if type(a[0]) == 'table' then
        vertCount = #a
        buffer = ffi.new('NxMeshVertexPosCoords[?]', vertCount, a)
    else
        vertCount = #a / 5 -- Five values: xyz, uv
        buffer = ffi.new('NxMeshVertexPosCoords[?]', vertCount)
        for i = 1, vertCount do
            local vertex = {}
            for j = 1, 5 do
                vertex[j] = a[(i-1) * 5 + j]
            end
            buffer[i-1] = ffi.new('NxMeshVertexPosCoords', vertex)
        end
    end

    return buffer, vertexSize * vertCount, vertCount
end

function Geometry.static.indexDataToBuffer(a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) ~= 'table' then return ffi.new('uint16_t*'), 0, 0 end

    local indexCount = #a
    return ffi.new('uint16_t[?]', indexCount, a), ffi.sizeof('uint16_t') * indexCount, indexCount
end

function Geometry:setVertexData(buffer, size, ...)
    if type(buffer) ~= 'cdata' then
        buffer, size = Geometry.vertexDataToBuffer(buffer, size, ...)
    end

    if buffer and size ~= 0 then
        self._vertexCount = size / vertexSize
        self._vertexBuffer = Arraybuffer.vertexbuffer(size, buffer)
    else
        self._vertexCount = 0
        self._vertexBuffer = nil
    end

    return self
end

function Geometry:setIndexData(buffer, size, ...)
    if type(buffer) ~= 'cdata' then
        buffer, size = Geometry.indexDataToBuffer(buffer, size, ...)
    end

    if buffer and size ~= 0 then
        self._indexCount = size / ffi.sizeof('uint16_t')
        self._indexBuffer = Arraybuffer.indexbuffer(size, buffer)
    else
        self._indexCount = 0
        self._indexBuffer = nil
    end

    return self
end

function Geometry:vertexCount()
    return self._vertexBuffer and self._vertexCount or 0
end

function Geometry:indexCount()
    return self._indexBuffer and self._indexBuffer or 0
end

function Geometry:_apply()
    if self._vertexBuffer then
        Arraybuffer.setVertexbuffer(self._vertexBuffer, 0, 0, vertexSize)
        Arraybuffer.setIndexbuffer(self._indexBuffer, 16)
        C.nxRendererSetVertexLayout(vertexLayout)
        return true
    end

    return false
end

return Geometry