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

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxVertexBuffer NxVertexBuffer;

    NxVertexBuffer* nxVertexBufferCreate();
    void nxVertexBufferRelease(NxVertexBuffer*);
    bool nxVertexBufferLoad(NxVertexBuffer*, void*, uint32_t, uint32_t);
    bool nxVertexBufferUpdate(NxVertexBuffer*, void*, uint32_t, uint32_t);
    uint32_t nxVertexBufferSize(const NxVertexBuffer*);
    uint32_t nxVertexBufferStride(const NxVertexBuffer*);
    uint32_t nxVertexBufferCount(const NxVertexBuffer*);
    uint32_t nxVertexBufferUsedMemory();
    void nxVertexBufferBind(NxVertexBuffer*, uint8_t, uint32_t);
]]

local class = require 'class'
local VertexBuffer = class 'graphics.vertexbuffer'

function VertexBuffer.static.usedMemory()
    return C.nxVertexBufferUsedMemory()
end

function VertexBuffer.static.bind(buffer, slot, offset)
    C.nxVertexBufferBind(buffer and buffer._cdata, slot or 0, offset or 0)
end

function VertexBuffer:initialize(data, size, stride)
    self._cdata = ffi.gc(C.nxVertexBufferCreate(), C.nxVertexBufferRelease)

    if data then self:load(data, size, stride) end
end

function VertexBuffer:release()
    C.nxVertexBufferReleaese(ffi.gc(self._cdata, nil))
end

function VertexBuffer:load(data, size, stride)
    C.nxVertexBufferLoad(self._cdata, data, size, stride)

    return self
end

function VertexBuffer:update(data, size, offset)
    C.nxVertexBufferUpdate(self._cdata, data, size or self:size(), offset or 0)

    return self
end

function VertexBuffer:size()
    return C.nxVertexBufferSize(self._cdata)
end

function VertexBuffer:stride()
    return C.nxVertexBufferStride(self._cdata)
end

function VertexBuffer:count()
    return C.nxVertexBufferCount(self._cdata)
end

function VertexBuffer:bind(slot, offset)
    VertexBuffer.bind(self, slot, offset)

    return self
end

return VertexBuffer
