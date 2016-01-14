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
    typedef struct NxIndexBuffer NxIndexBuffer;

    NxIndexBuffer* nxIndexBufferCreate();
    void nxIndexBufferRelease(NxIndexBuffer*);
    bool nxIndexBufferLoad(NxIndexBuffer*, void*, uint32_t, uint8_t);
    bool nxIndexBufferUpdate(NxIndexBuffer*, void*, uint32_t, uint32_t);
    uint32_t nxIndexBufferCount(const NxIndexBuffer*);
    uint32_t nxIndexBUfferSize(const NxIndexBuffer*);
    uint8_t nxIndexBufferFormat(const NxIndexBuffer*);
    uint32_t nxIndexBufferUsedMemory();
    void nxIndexBufferBind(NxIndexBuffer*);
]]

local class = require 'class'
local IndexBuffer = class 'graphics.indexbuffer'

local toFormat = {
    ['16'] = 0,
    ['32'] = 1
}
local fromFormat = {
    [0] = '16',
    [1] = '32'
}

function IndexBuffer.static.usedMemory()
    return C.nxIndexBufferUsedMemory()
end

function IndexBuffer.static.bind(buffer)
    C.nxIndexBufferBind(buffer and buffer._cdata)
end

function IndexBuffer:initialize(data, size, format)
    self._cdata = ffi.gc(C.nxIndexBufferCreate(), C.nxIndexBufferRelease)
    if data then self:load(data, size, format) end
end

function IndexBuffer:release()
    C.nxIndexBufferRelease(ffi.gc(self._cdata, nil))
end

function IndexBuffer:load(data, size, format)
    C.nxIndexBufferLoad(self._cdata, data, size, toFormat[format] or 0)
    return self
end

function IndexBuffer:update(data, size, offset)
    C.nxIndexBufferUpdate(self._cdata, data, size or self:size(), offset or 0)
    return self
end

function IndexBuffer:count()
    return C.nxIndexBufferCount(self._cdata)
end

function IndexBuffer:size()
    return C.nxIndexBufferSize(self._cdata)
end

function IndexBuffer:format()
    return fromFormat[C.nxIndexBufferFormat(self._cdata)]
end

function IndexBuffer:bind()
    IndexBuffer.bind(self)
end

return IndexBuffer
