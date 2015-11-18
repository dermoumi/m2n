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

------------------------------------------------------------
-- A parent of all classes that can be drawn on a 2d space
------------------------------------------------------------
local class = require 'nx.class'
local Entity2D = class 'nx.entity2d'

local Matrix = require 'nx.matrix'

------------------------------------------------------------
local State = class 'nx.entity2d.state'
Entity2D.static.State = State

------------------------------------------------------------
function State:initialize(transMatrix, r, g, b, a)
    self._transMatrix = transMatrix or Matrix:new()
    self._colR = r or 255
    self._colG = g or 255
    self._colB = b or 255
    self._colA = a or 255
end

------------------------------------------------------------
function State:clone()
    return State:new(self:matrix():clone(), self:color())
end

------------------------------------------------------------
function State:combineColor(r, g, b, a)
    self._colR = (self._colR / 255) * (r or 255)
    self._colG = (self._colG / 255) * (g or 255)
    self._colB = (self._colB / 255) * (b or 255)
    self._colA = (self._colA / 255) * (a or 255)
end

------------------------------------------------------------
function State:matrix()
    return self._transMatrix
end

------------------------------------------------------------
function State:color(normalize)
    if normalize then
        return self._colR / 255, self._colG / 255, self._colB / 255, self._colA / 255
    else
        return self._colR, self._colG, self._colB, self._colA
    end
end

------------------------------------------------------------
function Entity2D:initialize()
    self._posX,    self._posY    = 0, 0
    self._scaleX,  self._scaleY  = 1, 1
    self._originX, self._originY = 0, 0
    self._rotation = 0

    self._colR, self._colG, self._colB, self._colA = 255, 255, 255, 255
end

------------------------------------------------------------
function Entity2D:setPosition(x, y)
    self._posX, self._posY = x, y

    self._matrix = nil
end

------------------------------------------------------------
function Entity2D:setScale(x, y)
    self._scaleX, self._scaleY = x, y

    self._matrix = nil
end

------------------------------------------------------------
function Entity2D:setRotation(rad)
    self._rotation = rad % (math.pi * 2)

    self._matrix = nil
end

------------------------------------------------------------
function Entity2D:setOrigin(x, y)
    self._originX, self._originY = x, y

    self._matrix = nil
end

------------------------------------------------------------
function Entity2D:setColor(r, g, b, a)
    self._colR = r or 255
    self._colG = g or 255
    self._colB = b or 255
    self._colA = a or 255
end

------------------------------------------------------------
function Entity2D:position()
    return self._posX, self._posY
end

------------------------------------------------------------
function Entity2D:scale()
    return self._scaleX, self._scaleY
end

------------------------------------------------------------
function Entity2D:rotation()
    return self._rotation
end

------------------------------------------------------------
function Entity2D:origin()
    return self._originX, self._originY
end

------------------------------------------------------------
function Entity2D:color()
    return self._colR, self._colG, self._colB, self._colA
end

------------------------------------------------------------
function Entity2D:matrix()
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

    return self._matrix
end

------------------------------------------------------------
function Entity2D:_render(camera, state)
    -- Nothing to do
end

------------------------------------------------------------
function Entity2D:_draw(camera, state)
    state:matrix():combine(self:matrix())
    state:combineColor(self:color())

    self:_render(camera, state)
end

------------------------------------------------------------
return Entity2D