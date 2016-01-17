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

local ffi         = require 'ffi'
local Graphics    = require 'graphics'
local Scene       = require 'graphics.scene'

local Model = Scene:subclass 'graphics.model'

function Model:initialize(model)
    Scene.initialize(self, 'model')
    self.meshes = {}
    if model then self:setModel(model) end
end

function Model:setModel(model)
    return model:makeEntity(self)
end

function Model:attached(node)
    Scene.attached(self, node)
    if node.type == 'mesh' then
        local table = self.meshes[node.geometry]
        if not table then
            table = {}
            self.meshes[node.geometry] = table
        end

        table[#table+1] = node
    end
end

function Model:_render(camera, context)
    local proj, renderFunc = camera:projection(), nil
    for geometry, meshTable in pairs(self.meshes) do
        if geometry:_apply() then
            renderFunc = geometry._indexBuffer
                and ffi.C.nxRendererDrawIndexed
                or ffi.C.nxRendererDraw

            for i, mesh in pairs(meshTable) do
                if mesh.material and mesh.material:_apply(proj, mesh:matrix(true), context) then
                    renderFunc(4, mesh.start, mesh.count)
                end
            end
        end
    end
end

return Model
