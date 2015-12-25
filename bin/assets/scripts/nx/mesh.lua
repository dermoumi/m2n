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

local Renderer    = require 'nx.renderer'
local Arraybuffer = require 'nx.arraybuffer'
local Material    = require 'nx.material'
local Entity3D    = require 'nx.entity3d'
local class       = require 'nx.class'

local Mesh = class 'nx.mesh'
Mesh:include(Entity3D)

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        float x, y, z, u, v;
    } NxMeshVertexPosCoords;
]]

------------------------------------------------------------
local vertexSize = ffi.sizeof('NxMeshVertexPosCoords')

------------------------------------------------------------
function Mesh.static._vertexLayout()
    return Renderer.vertexLayout(3)
end

------------------------------------------------------------
function Mesh:initialize()
    Entity3D.initialize(self)

    self._materials = {
        ambient = Material:new()
    }
end

------------------------------------------------------------
function Mesh:setMaterial(material, context)
    self._materials[context or 'ambient'] = material

    return self
end

------------------------------------------------------------
function Mesh:material(context)
    return self._materials[context or 'ambient']
end

------------------------------------------------------------
function Mesh:setVertexData(a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) ~= 'table' then return self end

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

    return self
end

------------------------------------------------------------
function Mesh:setIndexData(a, b, ...)
    if b then a = {a, b, ...} end
    if type(a) ~= 'table' then return self end

    local buffer = ffi.new('uint16_t[?]', #a, a)

    self._indexCount = #a
    self._indexBuffer = Arraybuffer._indexbuffer(ffi.sizeof(buffer), buffer)

    return self
end

------------------------------------------------------------
function Mesh:_render(camera, context)
    local material = self._materials[context]
    if self._vertexBuffer and material then
        material:_apply(camera:matrix(), self:matrix())
        
        Arraybuffer.setVertexbuffer(self._vertexBuffer, 0, 0, vertexSize)
        Arraybuffer.setIndexbuffer(self._indexBuffer, 16)
        C.nxRendererSetVertexLayout(Mesh._vertexLayout())

        if self._indexBuffer then
            C.nxRendererDrawIndexed(4, 0, self._indexCount)
        else
            C.nxRendererDraw(4, 0, self._vertexCount)
        end
    end
end

------------------------------------------------------------
return Mesh