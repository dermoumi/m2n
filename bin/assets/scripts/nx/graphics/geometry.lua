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

local Graphics    = require 'nx.graphics'
local Arraybuffer = require 'nx.graphics.arraybuffer'
local class       = require 'nx.class'

local Geometry = class 'nx.graphics.geometry'

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        float x, y, z, u, v;
    } NxMeshVertexPosCoords;
]]

------------------------------------------------------------
local vertexSize, vertexLayout =
    ffi.sizeof('NxMeshVertexPosCoords'),
    Graphics.vertexLayout(3)

------------------------------------------------------------
function Geometry:setVertexData(a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) == 'table' then
        local buffer
        if type(a[0]) == 'table' then
            self._vertexCount = #a
            buffer = ffi.new('NxMeshVertexPosCoords[?]', self._vertexCount, a)
        else
            self._vertexCount = #a / 5 -- Five values: xyz, uv
            buffer = ffi.new('NxMeshVertexPosCoords[?]', self._vertexCount)
            for i = 1, self._vertexCount do
                local vertex = {}
                for j = 1, 5 do
                    vertex[j] = a[(i-1) * 5 + j]
                end
                buffer[i-1] = ffi.new('NxMeshVertexPosCoords', vertex)
            end
        end

        self._vertexBuffer = Arraybuffer.vertexbuffer(ffi.sizeof(buffer), buffer)
    else
        self._vertexBuffer = nil
    end

    return self
end

------------------------------------------------------------
function Geometry:setIndexData(a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) == 'table' then
        local buffer = ffi.new('uint16_t[?]', #a, a)

        self._indexCount = #a
        self._indexBuffer = Arraybuffer.indexbuffer(ffi.sizeof(buffer), buffer)
    else
        self._indexBuffer = nil
    end

    return self
end

------------------------------------------------------------
function Geometry:vertexCount()
    return self._vertexBuffer and self._vertexCount or 0
end

------------------------------------------------------------
function Geometry:indexCount()
    return self._indexBuffer and self._indexBuffer or 0
end

------------------------------------------------------------
function Geometry:_apply()
    if self._vertexBuffer then
        Arraybuffer.setVertexbuffer(self._vertexBuffer, 0, 0, vertexSize)
        Arraybuffer.setIndexbuffer(self._indexBuffer, 16)
        C.nxRendererSetVertexLayout(vertexLayout)
        return true
    end

    return false
end

------------------------------------------------------------
return Geometry