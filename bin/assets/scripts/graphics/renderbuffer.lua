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

local class   = require 'class'
local Config  = require 'config'
local Log     = require 'util.log'
local Texture = require 'graphics.texture'

local Renderbuffer = class 'graphics.renderbuffer'

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxRenderbuffer NxRenderbuffer;
    typedef struct NxTexture NxTexture;

    NxRenderbuffer* nxRenderbufferNew();
    void nxRenderbufferRelease(NxRenderbuffer*);
    uint8_t nxRenderbufferCreate(NxRenderbuffer*, uint8_t, uint16_t, uint16_t, bool, uint8_t,
        uint8_t);
    NxTexture* nxRenderbufferTexture(NxRenderbuffer*, uint8_t);
    void nxRenderbufferSize(const NxRenderbuffer*, uint16_t*);
    uint8_t nxRenderbufferFormat(const NxRenderbuffer*);
    void nxRenderbufferBind(const NxRenderbuffer*);
]]

------------------------------------------------------------
function Renderbuffer.static.bind(buffer)
    if buffer then buffer = buffer._cdata end
    C.nxRenderbufferBind(buffer)

    return Renderbuffer
end

------------------------------------------------------------
function Renderbuffer:initialize(width, height, depth, samples, numColBufs)
    self._textures = {}

    if width and height then
        self:create(width, height, depth, samples, numColBufs)
    end
end

------------------------------------------------------------
function Renderbuffer:release()
    if not self._cdata then return end

    C.nxRenderbufferRelease(ffi.gc(self._cdata, nil))
    self._cdata = nil
    self._textures = {}
end

------------------------------------------------------------
function Renderbuffer:bind()
    -- No cdata check as to allow debinding
    C.nxRenderbufferBind(self._cdata)

    return self
end

------------------------------------------------------------
function Renderbuffer:create(width, height, depth, samples, numColBufs)
    self:release()

    if depth == nil then depth = false end
    samples = samples or Config.multisamplingLevel
    numColBufs = numColBufs or 1

    local handle = C.nxRenderbufferNew()
    local status = C.nxRenderbufferCreate(handle, 1, width, height, depth, numColBufs, samples)

    if status == 1 then
        Log.warning('Cannot create renderbuffer: invalid size')
        C.nxRenderbufferRelease(handle)
    elseif status == 2 then
        Log.warning('Cannot create renderbuffer')
        C.nxRenderbufferRelease(handle)
    else
        self._cdata = ffi.gc(handle, C.nxRenderbufferRelease)
    end

    return self
end

------------------------------------------------------------
function Renderbuffer:texture(bufIndex)
    if self._cdata == nil then return Texture:new() end

    if bufIndex == 'depth' then
        bufIndex = 32
    elseif not bufIndex then
        bufIndex = 0
    end

    local texture = self._textures[bufIndex]

    if not texture then
        texture = Texture:allocate()
        texture._cdata = ffi.cast('NxTexture*', C.nxRenderbufferTexture(self._cdata, bufIndex))
        self._textures[bufIndex] = texture
    end

    return texture
end

------------------------------------------------------------
function Renderbuffer:size()
    local sizePtr = ffi.new('uint16_t[2]')
    C.nxRenderbufferSize(self._cdata, sizePtr)

    return tonumber(sizePtr[0]), tonumber(sizePtr[1])
end

------------------------------------------------------------
return Renderbuffer