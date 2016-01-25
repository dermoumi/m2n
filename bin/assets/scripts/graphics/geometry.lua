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
local class        = require 'class'

local Geometry = class 'graphics.geometry'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        float x, y, z, u, v, nx, ny, nz;
    } NxMeshVertexPosCoords;
]]

local formats = {
    {
        stride = { ffi.sizeof('NxMeshVertexPosCoords') },
        layout = Graphics.vertexLayout(3)
    }
}

function Geometry.static.factory(task)
    task:addTask(function(geom, filename)
            local success = true

            local file = require('filesystem.inputfile'):new(filename)
                :onError(function(err)
                        local Log = require 'util.log'
                        Log.error('Unable to load geometry file: ' .. err)
                        success = false
                    end)

            local headGuard = ffi.string(file:read(6), 6)
            if headGuard == 'M2N1.0' then
                local format = file:readU8() + 1
                if format == 1 then
                    local vertBuffer, vertBufSize, indexBuffer, indexBufSize

                    vertBufSize, indexBufSize = file:readU32(), file:readU32()
                    if vertBufSize > 0 then vertBuffer = file:read(vertBufSize) end
                    if indexBufSize > 0 then indexBuffer = file:read(indexBufSize) end

                    if not success then return false end

                    geom:setFormat(1)
                        :setVertexData(1, vertBuffer, vertBufSize)
                        :setIndexData(indexBuffer, indexBufSize)
                else
                    error('Unknown geometry format')
                end
            else
                error('Unsupported geometry file')
            end
        end)
end

function Geometry:initialize()
    self._format = formats[0]
    self._vertexBuffers = {}
end

function Geometry:setFormat(format)
    self._format = formats[format]

    return self
end

function Geometry:setVertexData(slot, buffer, size)
    if buffer and size ~= 0 then
        self._vertexBuffers[slot] = VertexBuffer:new(buffer, size, self._format.stride[slot])
    else
        self._vertexBuffers[slot] = nil
    end

    return self
end

function Geometry:setIndexData(buffer, size)
    if buffer and size ~= 0 then
        self._indexBuffer = IndexBuffer:new(buffer, size, '16')
    else
        self._indexBuffer = nil
    end

    return self
end

function Geometry:vertexCount(slot)
    return self._vertexBuffers[slot or 1] and self._vertexBuffers[slot or 1]:count() or 0
end

function Geometry:indexCount()
    return self._indexBuffer and self._indexBuffer:count() or 0
end

function Geometry:_apply()
    local applied = false

    for i, buffer in ipairs(self._vertexBuffers) do
        buffer:bind(i-1)
        applied = true
    end

    if applied then
        IndexBuffer.bind(self._indexBuffer, 16)
        C.nxRendererSetVertexLayout(self._format.layout)
        return true
    else
        return false
    end
end

return Geometry