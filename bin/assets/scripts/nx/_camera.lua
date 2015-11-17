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
-- A base class for camera objects
------------------------------------------------------------
local class = require 'nx.class'
local Camera = class 'nx._camera'

local Window   = require 'nx.window'
local Renderer = require 'nx.renderer'
local Renderbuffer = require 'nx.renderbuffer'

local ffi = require 'ffi'
local C   = ffi.C

------------------------------------------------------------
function Camera:setViewport(left, top, width, height)
    self._vpX = left
    self._vpY = top
    self._vpW = width
    self._vpH = height
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
function Camera:setRenderbuffer(rb)
    self._rb = rb
    self._needUpdate = true
end

------------------------------------------------------------
function Camera:renderbuffer()
    return self._rb
end

------------------------------------------------------------
function Camera:matrix()
    return require('nx.matrix'):new()
end

------------------------------------------------------------
function Camera:clear(r, g, b, a, depth, col0, col1, col2, col3, clearDepth)
    self:_setupDrawing()

    -- Make sure of values
    if col0 == nil then col0 = true end
    if col1 == nil then col1 = true end
    if col2 == nil then col2 = true end
    if col3 == nil then col3 = true end
    if clearDepth == nil then clearDepth = true end

    C.nxRendererClear(
        r or 0, g or 0, b or 0, a or 255, depth or 1.0, col0, col1, col2, col3, clearDepth
    )
end

------------------------------------------------------------
function Camera:draw(drawable)
    self:_setupDrawing()

    drawable:_render(self, self:matrix():clone(), 255, 255, 255, 255)
end

------------------------------------------------------------
function Camera:_setupDrawing()
    if not self._needUpdate then return end
    self._needUpdate = false

    -- Setup renderbuffer
    Renderbuffer.setCurrent(self._rb)

    -- Setup viewport
    C.nxRendererSetViewport(self:viewport())
end

------------------------------------------------------------
return Camera