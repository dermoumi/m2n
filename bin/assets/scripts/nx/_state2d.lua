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

local Matrix = require 'nx.matrix'
local class  = require 'nx.class'

local State2D = class 'nx._state2d'

------------------------------------------------------------
function State2D:initialize(transMatrix, r, g, b, a, blendSrc, blendDst)
    self._transMatrix = transMatrix or Matrix:new()
    self._colR = r or 255
    self._colG = g or 255
    self._colB = b or 255
    self._colA = a or 255

    self._blendSrc, self._blendDst = blendSrc or 'alpha', blendDst
end

------------------------------------------------------------
function State2D:clone()
    return State2D:new(
        self:matrix():clone(),
        self._corR, self._colG, self._colB, self._colA,
        self._blendSrc, self._blendDst
    )
end

------------------------------------------------------------
function State2D:combineMatrix(mat)
    self._transMatrix:combine(mat)
end

------------------------------------------------------------
function State2D:combineColor(r, g, b, a)
    self._colR = self._colR * r / 255
    self._colG = self._colG * g / 255
    self._colB = self._colB * b / 255
    self._colA = self._colA * (a or 255) / 255
end

------------------------------------------------------------
function State2D:matrix()
    return self._transMatrix
end

------------------------------------------------------------
function State2D:color(normalize)
    if normalize then
        return self._colR / 255, self._colG / 255, self._colB / 255, self._colA / 255
    else
        return self._colR, self._colG, self._colB, self._colA
    end
end

------------------------------------------------------------
function State2D:blendMode()
    return self._blendSrc, self._blendDst
end

------------------------------------------------------------
return State2D