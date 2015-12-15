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
        uint32_t id;
    } NxArrayBuffer;
]]

------------------------------------------------------------
-- A class to handle VRAM buffers creation and managment
------------------------------------------------------------
local class = require 'nx.class'
local Arraybuffer = class 'nx.arraybuffer'

local Renderer = require 'nx.renderer'

------------------------------------------------------------
local function destroyBuffer(cdata)
    C.nxRendererDestroyBuffer(cdata.id)
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
    return C.nxRendererGetBufferMemory()
end

------------------------------------------------------------
function Arraybuffer.static.setVertexbuffer(buffer, slot, offset, stride)
    buffer = buffer and buffer._cdata.id or 0
    C.nxRendererSetVertexBuffer(slot, buffer, offset, stride)
end

------------------------------------------------------------
function Arraybuffer.static.setIndexbuffer(buffer, format)
    buffer = buffer and buffer._cdata.id or 0
    if format ~= 16 and format ~= 32 then format = 16 end

    C.nxRendererSetIndexBuffer(buffer, format / 16 - 1)
end

------------------------------------------------------------
function Arraybuffer:initialize()
    self._cdata = ffi.new('NxArrayBuffer', {0})
end

------------------------------------------------------------
function Arraybuffer:release()
    C.nxRendererDestroyBuffer(ffi.gc(self._cdata, nil).id)
end

------------------------------------------------------------
function Arraybuffer:createVertexbuffer(size, data)
    self._cdata = ffi.new('NxArrayBuffer', {C.nxRendererCreateVertexBuffer(size, data)})
    ffi.gc(self._cdata, destroyBuffer)

    return self._cdata.id ~= 0
end

------------------------------------------------------------
function Arraybuffer:createIndexbuffer(size, data)
    self._cdata = ffi.new('NxArrayBuffer', {C.nxRendererCreateIndexBuffer(size, data)})
    ffi.gc(self._cdata, destroyBuffer)

    return self._cdata.id ~= 0
end

------------------------------------------------------------
function Arraybuffer:setData(offset, size, data)
    return C.nxRendererUpdateBufferData(self._cdata.id, offset, size, data)
end

------------------------------------------------------------
function Arraybuffer:nativeHandle()
    return self._cdata.id
end

------------------------------------------------------------
return Arraybuffer