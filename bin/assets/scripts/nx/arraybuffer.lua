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

local Log      = require 'nx.log'
local Renderer = require 'nx.renderer'
local class    = require 'nx.class'

local Arraybuffer = class 'nx.arraybuffer'

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

------------------------------------------------------------
local function destroyBuffer(cdata)
    C.nxRendererDestroyBuffer(cdata[0])
end

------------------------------------------------------------
function Arraybuffer.static.vertexbuffer(size, data)
    return Arraybuffer:new():createVertexbuffer(size, data)
end

------------------------------------------------------------
function Arraybuffer.static.indexbuffer(size, data)
    return Arraybuffer:new():createIndexbuffer(size, data)
end

------------------------------------------------------------
function Arraybuffer.static.usedMemory()
    return C.nxRendererGetBufferMemory()
end

------------------------------------------------------------
function Arraybuffer.static.setVertexbuffer(buffer, slot, offset, stride)
    buffer = buffer and buffer._cdata[0] or 0
    C.nxRendererSetVertexBuffer(slot, buffer, offset, stride)

    return Arraybuffer
end

------------------------------------------------------------
function Arraybuffer.static.setIndexbuffer(buffer, format)
    buffer = buffer and buffer._cdata[0] or 0
    if format ~= 16 and format ~= 32 then format = 16 end

    C.nxRendererSetIndexBuffer(buffer, format / 16 - 1)

    return Arraybuffer
end

------------------------------------------------------------
function Arraybuffer:initialize()
    self._cdata = ffi.new('uint32_t[1]', {0})
end

------------------------------------------------------------
function Arraybuffer:release()
    destroyBuffer(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

------------------------------------------------------------
function Arraybuffer:createVertexbuffer(size, data)
    self._cdata = ffi.new('uint32_t[1]', {C.nxRendererCreateVertexBuffer(size, data)})
    ffi.gc(self._cdata, destroyBuffer)

    if self._cdata[0] == 0 then
        Log.warning('Failed to create vertex buffer')
    end

    return self
end

------------------------------------------------------------
function Arraybuffer:createIndexbuffer(size, data)
    self._cdata = ffi.new('uint32_t[1]', {C.nxRendererCreateIndexBuffer(size, data)})
    ffi.gc(self._cdata, destroyBuffer)

    if self._cdata[0] == 0 then
        Log.warning('Failed to create index buffer')
    end

    return self
end

------------------------------------------------------------
function Arraybuffer:setData(offset, size, data)
    if not C.nxRendererUpdateBufferData(self._cdata[0], offset, size, data) then
        Log.warning('Failed to set buffer data')
    end

    return self
end

------------------------------------------------------------
return Arraybuffer