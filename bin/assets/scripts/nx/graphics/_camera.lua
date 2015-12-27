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

local class        = require 'nx.class'
local Graphics     = require 'nx.graphics'
local Window       = require 'nx.window'
local Renderbuffer = require 'nx.graphics.renderbuffer'
local Matrix       = require 'nx.util.matrix'

local Camera = class 'nx.graphics._camera'

------------------------------------------------------------
local ffi = require 'ffi'
local C   = ffi.C

------------------------------------------------------------
function Camera:_invalidate()
    self._projection = nil
    self._invProjection = nil

    return self
end

------------------------------------------------------------
function Camera:setViewport(left, top, width, height)
    self._vpX = left
    self._vpY = top
    self._vpW = width
    self._vpH = height

    return self
end

------------------------------------------------------------
function Camera:setRenderbuffer(rb)
    self._rb = rb
    self._updated = false

    return self
end

------------------------------------------------------------
function Camera:viewport()
    if self._vpX then
        return self._vpX, self._vpY, self._vpW, self._vpH
    elseif self._rb then
        return 0, 0, self._rb:size()
    else
        return 0, 0, Window:size()
    end
end

------------------------------------------------------------
function Camera:renderbuffer()
    return self._rb
end

------------------------------------------------------------
function Camera:projection()
    return Matrix:new()
end

------------------------------------------------------------
function Camera:invProjection()
    if not self._invProjection then
        self._invProjection = self:projection():inverse()
    end

    return self._invProjection
end

------------------------------------------------------------
function Camera:clear(r, g, b, a, depth, col0, col1, col2, col3, clearDepth)
    self:apply()

    -- Make sure the values are valid
    if r == nil then r, g, b = 0, 0, 0 end
    if col0 == nil then col0, col1, col2, col3 = true, true, true, true end
    if clearDepth == nil then clearDepth = true end

    C.nxRendererClear(
        r, g, b, a or 255, depth or 1.0, col0, col1, col2, col3, clearDepth
    )

    return self
end

------------------------------------------------------------
function Camera:apply()
    -- Setup viewport
    C.nxRendererSetViewport(self:viewport())

    if not self._updated then
        Renderbuffer.bind(self._rb)

        self._updated = true
    end

    return self
end

------------------------------------------------------------
return Camera