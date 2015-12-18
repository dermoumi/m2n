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

local BinaryFile = require 'nx._binaryfile'
local OutputFile = BinaryFile:subclass('nx.outputfile')

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    typedef struct PHYSFS_File PHYSFS_File;

    const char* nxFsGetError();

    PHYSFS_File* nxFsOpenWrite(const char*);
    bool nxFsFlush(PHYSFS_File*);
    void nxFsClose(PHYSFS_File*);

    bool nxFsWrite(PHYSFS_File*, const char*, size_t, size_t*);
    bool nxFsWriteS8(PHYSFS_File*, int8_t);
    bool nxFsWriteS16(PHYSFS_File*, int16_t);
    bool nxFsWriteS32(PHYSFS_File*, int32_t);
    bool nxFsWriteS64(PHYSFS_File*, int64_t);
    bool nxFsWriteU8(PHYSFS_File*, uint8_t);
    bool nxFsWriteU16(PHYSFS_File*, uint16_t);
    bool nxFsWriteU32(PHYSFS_File*, uint32_t);
    bool nxFsWriteS64(PHYSFS_File*, uint64_t);
    bool nxFsWriteFloat(PHYSFS_File*, double);
    bool nxFsWriteDouble(PHYSFS_File*, double);
    bool nxFsWriteString(PHYSFS_File*, const char*);
]]

------------------------------------------------------------
function OutputFile:initialize(filename)
    BinaryFile.initialize(self, filename)
end

------------------------------------------------------------
function OutputFile:open(filename)
    -- If already open, close it
    if self:isOpen() then self:close() end

    local handle = C.nxFsOpenWrite(filename)
    if handle == nil then return self:_throwError(self) end

    self._cdata = ffi.gc(handle, C.nxFsClose)
    return self
end

------------------------------------------------------------
function OutputFile:write(buffer, size)
    if self._cdata ~= nil then
        -- If no size supplied, assume size of buffer string
        if not size then size = #buffer end

        local bytesWrittenPtr = ffi.new('size_t[1]')

        local ok = C.nxFsWrite(self._cdata, buffer, size, bytesWrittenPtr)
        if not ok then self:_throwError() end
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeS8(val)
    if self._cdata ~= nil and not C.nxFsWriteS8(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeS16(val)
    if self._cdata ~= nil and not C.nxFsWriteS16(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeS32(val)
    if self._cdata ~= nil and not C.nxFsWriteS32(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeS64(val)
    if self._cdata ~= nil and not C.nxFsWriteS64(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeU8(val)
    if self._cdata ~= nil and not C.nxFsWriteU8(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeU16(val)
    if self._cdata ~= nil and not C.nxFsWriteU16(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeU32(val)
    if self._cdata ~= nil and not C.nxFsWriteU32(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeU64(val)
    if self._cdata ~= nil and not C.nxFsWriteU64(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeFloat(val)
    if self._cdata ~= nil and not C.nxFsWriteFloat(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeDouble(val)
    if self._cdata ~= nil and not C.nxFsWriteDouble(self._cdata, val) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
function OutputFile:writeString(str)
    if self._cdata ~= nil and not C.nxFsWriteString(self._cdata, str) then
        self:_throwError()
    end

    return self
end

------------------------------------------------------------
return OutputFile