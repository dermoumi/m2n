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

local Mesh     = require 'graphics.mesh'
local Material = require 'graphics.material'
local class    = require 'class'

local Model = class 'graphics.model'

function Model:initialize()
    self._meshes = {}
end

function Model:addMesh(m, start, count)
    if start then m = Mesh:new(m or Material:new(), start, count) end

    self._meshes[#self._meshes+1] = m

    return self
end

function Model:setGeometry(geometry)
    self._geometry = geometry

    return self
end

function Model:_draw(projMat, transMat, context)
    if #self._meshes > 0 and self._geometry and self._geometry:_apply() then
        local indexed = self._geometry._indexBuffer
        for i, mesh in ipairs(self._meshes) do
            mesh:_draw(projMat, transMat, context, indexed)
        end
    end
end

return Model