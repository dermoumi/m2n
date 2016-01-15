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

local class   = require 'class'
local Config  = require 'config'
local Log     = require 'util.log'
local Texture = require 'graphics.texture'

local RenderBuffer = class 'graphics.renderbuffer'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxRenderbuffer NxRenderBuffer;
    typedef struct NxTexture NxTexture;

    NxRenderBuffer* nxRenderBufferNew();
    void nxRenderBufferRelease(NxRenderBuffer*);
    bool nxRenderBufferCreate(NxRenderBuffer*, uint8_t, uint16_t, uint16_t, bool, uint8_t,
        uint8_t);
    NxTexture* nxRenderBufferTexture(NxRenderBuffer*, uint8_t);
    void nxRenderBufferSize(const NxRenderBuffer*, uint16_t*);
    uint8_t nxRenderBufferFormat(const NxRenderBuffer*);
    void nxRenderBufferBind(const NxRenderBuffer*);
]]

function RenderBuffer.static.bind(buffer)
    C.nxRenderBufferBind(buffer and buffer._cdata)

    return RenderBuffer
end

function RenderBuffer:initialize(width, height, depth, samples, numColBufs)
    self._cdata = ffi.gc(C.nxRenderBufferNew(), C.nxRenderBufferRelease)
    self._textures = {}

    if width and height then
        self:create(width, height, depth, samples, numColBufs)
    end
end

function RenderBuffer:release()
    self._textures = {}
    C.nxRenderBufferRelease(ffi.gc(self._cdata, nil))
end

function RenderBuffer:bind()
    -- No cdata check as to allow debinding
    C.nxRenderBufferBind(self._cdata)

    return self
end

function RenderBuffer:create(width, height, depth, samples, numColBufs)
    C.nxRenderBufferCreate(
        self._cdata, 1, width, height, not not depth, numColBufs or 1,
        samples or Config.multisamplingLevel
    )
    self._textures = {}

    return self
end

function RenderBuffer:texture(bufIndex)
    if bufIndex == 'depth' then
        bufIndex = 32
    elseif not bufIndex then
        bufIndex = 0
    end

    local texture = self._textures[bufIndex]

    if not texture then
        texture = Texture:allocate()
        texture._cdata = ffi.cast('NxTexture*', C.nxRenderBufferTexture(self._cdata, bufIndex))
        self._textures[bufIndex] = texture
    end

    return texture
end

function RenderBuffer:size()
    local sizePtr = ffi.new('uint16_t[2]')
    C.nxRenderBufferSize(self._cdata, sizePtr)

    return tonumber(sizePtr[0]), tonumber(sizePtr[1])
end

return RenderBuffer