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
    typedef struct NxArraybuffer NxArraybuffer;

    NxArraybuffer* nxArraybufferNew();
    void nxArraybufferRelease(NxArraybuffer*);
    bool nxArraybufferCreateVertex(NxArraybuffer*, uint32_t, const void*);
    bool nxArraybufferCreateIndex(NxArraybuffer*, uint32_t, const void*);
    bool nxArraybufferSetData(NxArraybuffer*, uint32_t, uint32_t, const void*);
    uint32_t nxArraybufferUsedMemory();
    void nxArraybufferSetVertexbuffer(const NxArraybuffer*, uint8_t, uint32_t, uint32_t);
    void nxArraybufferSetIndexbuffer(const NxArraybuffer*, uint8_t);
]]

------------------------------------------------------------
-- A class to handle VRAM buffers creation and managment
------------------------------------------------------------
local class = require 'nx.class'
local Arraybuffer = class 'nx.arraybuffer'
------------------------------------------------------------
function Arraybuffer.static._fromCData(cdata)
    local buffer = Arraybuffer:allocate()
    buffer._cdata = ffi.cast('NxArraybuffer*', cdata)
    return buffer
end

------------------------------------------------------------
function Arraybuffer.static.vertexbuffer(size, data)
    local buffer = Arraybuffer:new()

    ok, err = buffer:createVertexbuffer(size, data)
    if not ok then return nil, err end

    return buffer
end

------------------------------------------------------------
function Arraybuffer.static.indexbuffer(size, data)
    local buffer = Arraybuffer:new()

    ok, err = buffer:createIndexbuffer(size, data)
    if not ok then return nil, err end

    return buffer
end

------------------------------------------------------------
function Arraybuffer.static.usedMemory()
    return C.nxArraybufferUsedMemory()
end

------------------------------------------------------------
function Arraybuffer.static.setVertexbuffer(buffer, slot, offset, stride)
    if buffer then buffer = buffer._cdata end

    C.nxArraybufferSetVertexbuffer(buffer, slot, offset, stride)
end

------------------------------------------------------------
function Arraybuffer.static.setIndexbuffer(buffer, format)
    if buffer then buffer = buffer._cdata end
    if format ~= 16 and format ~= 32 then format = 16 end

    C.nxArraybufferSetIndexbuffer(buffer, format / 16 - 1)
end

------------------------------------------------------------
function Arraybuffer:initialize()
    local handle = C.nxArraybufferNew();
    self._cdata = ffi.gc(handle, C.nxArraybufferRelease)
end

------------------------------------------------------------
function Arraybuffer:release()
    if self._cdata == nil then return end
    C.nxArraybufferRelease(ffi.gc(self._cdata, nil))
end

------------------------------------------------------------
function Arraybuffer:createVertexbuffer(size, data)
    if not C.nxArraybufferCreateVertex(self._cdata, size, data) then
        return false, 'Unable to create vertex buffer'
    end

    return true
end

------------------------------------------------------------
function Arraybuffer:createIndexbuffer(size, data)
    if not C.nxArraybufferCreateIndex(self._cdata, size, data) then
        return false, 'Unable to create index buffer'
    end

    return true
end

------------------------------------------------------------
function Arraybuffer:setData(offset, size, data)
    if not C.nxArraybufferSetData(self._cdata, offset, size, data) then
        return false, 'Unable to set array buffer data'
    end

    return true
end

------------------------------------------------------------
return Arraybuffer