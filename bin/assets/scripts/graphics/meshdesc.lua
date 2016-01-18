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

local Material    = require 'graphics.material'
local SceneDesc = require 'graphics.scenedesc'

local MeshDesc = SceneDesc:subclass 'graphics.meshdesc'

function MeshDesc:initialize()
    SceneDesc.initialize(self, 'mesh')
end

function MeshDesc:setGeometry(geom, start, count)
    self.geometry = geom
    self.start = start or 0
    self.count = count or ((geom._indexBuffer and geom:indexCount() or geom:vertexCount()) - start)

    return self
end

function MeshDesc:setMaterial(material)
    self.material = material
    return self
end

function MeshDesc:makeEntity(entity)
    entity = entity or require('graphics.mesh'):new()

    entity.geometry, entity.start, entity.count, entity.material =
        self.geometry, self.start, self.count, self.material or Material:new()

    return SceneDesc.makeEntity(self, entity)
end

function MeshDesc:_validate()
    self.start = self.start or 0
    if not self.count then
        if self.geometry._indexBuffer then
            self.count = self.geometry:indexCount() - self.start
        else
            self.count = self.geometry:vertexCount() - self.start
        end
    end
end

return MeshDesc
