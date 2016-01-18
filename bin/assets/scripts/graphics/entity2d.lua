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

local Graphics = require 'graphics'
local Matrix   = require 'util.matrix'
local Node     = require 'graphics.node'

local Entity2D = Node:subclass 'graphics.entity2d'

function Entity2D:initialize()
    Node.initialize(self)

    self._posX,    self._posY    = 0, 0
    self._scaleX,  self._scaleY  = 1, 1
    self._originX, self._originY = 0, 0
    self._rotation = 0

    self._colR, self._colG, self._colB, self._colA = 1, 1, 1, 1
end

function Entity2D:_markDirty()
    self._matrix = nil

    if self._absMatrix then
        self._absMatrix = nil
        for name, child in pairs(self.children) do
            child:_markDirty()
        end
    end

    return self
end

function Entity2D:attachedTo(node)
    self._absMatrix = nil
end

function Entity2D:detachedFrom(node)
    self._absMatrix = nil
end

function Entity2D:setPosition(x, y)
    if x then self._posX = x end
    if y then self._posY = y end

    return self:_markDirty()
end

function Entity2D:setScaling(x, y)
    if x then self._scaleX = x end
    if y then self._scaleY = y end

    return self:_markDirty()
end

function Entity2D:setRotation(rad)
    self._rotation = rad % (math.pi * 2)

    return self:_markDirty()
end

function Entity2D:setOrigin(x, y)
    if x then self._originX = x end
    if y then self._originY = y end

    return self:_markDirty()
end

function Entity2D:setColor(r, g, b, a)
    if r then self._colR = r/255 end
    if g then self._colG = g/255 end
    if b then self._colB = b/255 end
    if a then self._colA = a/255 end

    return self
end

function Entity2D:setBlendMode(blendSrc, blendDst)
    self._blendSrc, self._blendDst = blendSrc, blendDst
end

function Entity2D:position(absolute)
    if absolute and self.parent then
        local x, y, z = self.parent:matrix(true):apply(self._posX, self._posY, 0)
        return x, y
    end

    return self._posX, self._posY
end

function Entity2D:scaling(absolute)
    if absoulte and self.parent then
        local x, y = self.parent:scaling(true)
        return self._scaleX * x, self._scaleY * y
    end

    return self._scaleX, self._scaleY
end

function Entity2D:rotation(absolute)
    if absolute and self.parent then
        local rot = self.parent:rotation(true)
        return self._rotation + rot
    end

    return self._rotation
end

function Entity2D:origin(absolute)
    if absolute and self.parent then
        local x, y, z = self.parent:matrix(true):apply(self._originX, self._originY, 0)
        return x, y
    end

    return self._originX, self._originY
end

function Entity2D:move(x, y)
    return self:setPosition(self._posX + x, self._posY)
end

function Entity2D:rotate(rad)
    return self:setRotation(self._rotation + rad)
end

function Entity2D:scale(x, y)
    return self:setScaling(self._scaleX * x, self._scaleY * y)
end

function Entity2D:offset(x, y)
    return self:setOrigin(self._originX + x, self._originY + y)
end

function Entity2D:color(normalize, absolute)
    local r, g, b, a = self._colR, self._colG, self._colB, self._colA

    if absolute and self.parent then
        local pR, pG, pB, pA = self.parent:color(true, true)
        r, g, b, a = r * pR, g * pG, b * pB, a * pA
    end

    if normalize then
        return r, g, b, a
    else
        return r*255, g*255, b*255, a*255
    end
end

function Entity2D:blendMode()
    return self._blendSrc or 'alpha', self._blendDst
end

function Entity2D:matrix(absolute)
    if not self._matrix then
        local cos = math.cos(-self._rotation)
        local sin = math.sin(-self._rotation)
        local sxc = self._scaleX * cos
        local syc = self._scaleY * cos
        local sxs = self._scaleX * sin
        local sys = self._scaleY * sin
        local tx  = self._posX - self._originX * sxc - self._originY * sys
        local ty  = self._posY + self._originX * sxs - self._originY * syc

        self._matrix = Matrix:new()
        local m = self._matrix._cdata
        m[0], m[4], m[12] =  sxc, sys, tx
        m[1], m[5], m[13] = -sxs, syc, ty
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

function Entity2D:_render(camera, context)
    -- Nothing to do
end

function Entity2D:_draw(camera, context)
    Graphics.setBlendMode(self:blendMode())

    self:_render(camera, context)

    for name, child in pairs(self.children) do
        camera:draw(child, context)
    end
end

return Entity2D
