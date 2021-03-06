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

local Matrix     = require 'util.matrix'
local Quaternion = require 'util.quaternion'
local Node       = require 'graphics.node'

local Entity3D = Node:subclass 'graphics.entity3d'

function Entity3D:initialize(type)
    Node.initialize(self)
    
    self._posX, self._posY, self._posZ = 0, 0, 0
    self._scaleX, self._scaleY, self._scaleZ = 1, 1, 1
    self._quat = Quaternion:new(0, 0, 0)
    self.type = type
end

function Entity3D:_markDirty()
    self._matrix = nil

    if self._absMatrix then
        self._absMatrix = nil
        for name, child in pairs(self.children) do
            child:_markDirty()
        end
    end

    return self
end

function Entity3D:attachedTo(node)
    self._absMatrix = nil
end

function Entity3D:detachedFrom(node)
    self._absMatrix = nil
end

function Entity3D:boundingBox()
    return 0, 0, 0, 0, 0, 0
end

function Entity3D:lodLevel(eyeX, eyeY, eyeZ)
    return 0
end

function Entity3D:setPosition(x, y, z)
    if x then self._posX = x end
    if y then self._posY = y end
    if z then self._posZ = z end

    return self:_markDirty()
end

function Entity3D:setRotation(x, y, z, w)
    self._quat = Quaternion:new(x, y, z, w)

    return self:_markDirty()
end

function Entity3D:setScaling(x, y, z)
    if x then self._scaleX = x end
    if y then self._scaleY = y end
    if z then self._scaleZ = z end

    return self:_markDirty()
end

function Entity3D:move(x, y, z)
    return self:setPosition(self._posX + x, self._posY + y, self._posZ + z)
end

function Entity3D:rotate(x, y, z, w)
    self._quat:combine(not y and x or Quaternion:new(x, y, z, w))

    return self:_markDirty()
end

function Entity3D:scale(x, y, z)
    return self:setScale(self._scaleX * x, self._scaleY * y, self._scaleZ * z)
end

function Entity3D:position(absolute)
    if absolute and self.parent then
        return self.parent:matrix(true):apply(self._posX, self._posY, self._posZ)
    end

    return self._posX, self._posY, self._posZ
end

function Entity3D:quaternion(absolute)
    if absolute and self.parent then
        return Quaternion.combine(self.parent:quaternion(true), self._quat)
    end

    return self._quat
end

function Entity3D:rotation(absolute)
    return self:quaternion(absolute):angles()
end

function Entity3D:scaling(absolute)
    if absolute and self.parent then
        local x, y, z = self.parent:scaling(true)
        return self._scaleX * x, self._scaleY * y, self._scaleZ * z
    end

    return self._scaleX, self._scaleY, self._scaleZ
end

function Entity3D:matrix(absolute)
    -- Calculate relative transformation
    if not self._matrix then
        self._matrix = Matrix.fromTransformation(
            self._quat,
            self._posX, self._posY, self._posZ,
            self._scaleX, self._scaleY, self._scaleZ
        )
    end

    if absolute then
        if not self._absMatrix then
            self._absMatrix = self.parent
                and Matrix.fastMult43(self.parent:matrix(true), self._matrix)
                or  self._matrix
        end

        return self._absMatrix
    else
        return self._matrix
    end
end

function Entity3D:_render(camera, context)
    -- Nothing to do
end

function Entity3D:_draw(camera, context)
    self:_render(camera, context)

    for name, child in pairs(self.children) do
        camera:draw(child, context)
    end
end

return Entity3D
