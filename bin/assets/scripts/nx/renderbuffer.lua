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
-- ffi C declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        uint32_t rb;
        uint16_t width;
        uint16_t height;
        bool     depth;
        uint8_t  numColBufs;
        uint8_t  samples;
    } NxRenderbuffer;
]]

------------------------------------------------------------
-- Represents a renderable texture
------------------------------------------------------------
local class = require 'nx.class'
local Renderbuffer = class 'nx.renderbuffer'

local Renderer = require 'nx.renderer'

------------------------------------------------------------
local function destroy(cdata)
    if cdata.rb ~= 0 then
        C.nxRendererDestroyRenderbuffer(cdata.rb)
    end

    C.free(cdata)
end

------------------------------------------------------------
function Renderbuffer.static._fromCData(cdata)
    local rb = Renderbuffer:allocate()
    rb._cdata = ffi.cast('NxRenderbuffer*', cdata)
    rb._textures = {}
    return rb 
end

------------------------------------------------------------
function Renderbuffer:initialize(width, height, depth, samples, numColBufs)
    local handle = ffi.cast('NxRenderbuffer*', C.malloc(ffi.sizeof('NxRenderbuffer')))

    handle.rb         = 0
    handle.width      = 0
    handle.height     = 0
    handle.depth      = false
    handle.numColBufs = 0
    handle.samples    = 0
    self._cdata = ffi.gc(handle, destroy)

    self._textures = {}

    if width and height then
        ok, err = self:create(width, height, depth, samples, numColBufs)
        if not ok then
            return nil, err
        end
    end
end

------------------------------------------------------------
function Renderbuffer:release()
    destroy(ffi.gc(self._cdata, nil))
end

------------------------------------------------------------
function Renderbuffer:create(width, height, depth, samples, numColBufs)
    if not width or not height or width == 0 or height == 0 then
        return false, 'Cannot create renderbuffer: invalid dimensions'
    end

    if depth == nil then depth = false end
    samples = samples or 0 -- TODO: fetch from settings
    numColBufs = numColBufs or 1

    local c = self._cdata
    c.rb = C.nxRendererCreateRenderbuffer(width, height, 1, depth, numColBufs, samples)

    if c.rb == 0 then
        return false, 'Cannot create renderbuffer'
    end

    c.width = width
    c.height = height
    c.depth = depth
    c.numColBufs = numColBufs
    c.samples = samples

    return true
end

------------------------------------------------------------
function Renderbuffer:texture(bufIndex)
    if bufIndex == 'depth' then
        bufIndex = 32
    elseif not bufIndex or bufIndex >= self._cdata.numColBufs then
        bufIndex = 0
    end

    local texture = self._textures[bufIndex]

    if not texture then
        texture = require('nx.texture')._fromRenderbuffer(self, bufIndex)
        self._textures[bufIndex] = texture
    end

    return texture
end

------------------------------------------------------------
return Renderbuffer