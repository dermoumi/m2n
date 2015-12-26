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

local BinaryFile = require 'nx.filesystem._binaryfile'

local InputFile = BinaryFile:subclass('nx.filesystem.inputfile')

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    typedef struct PHYSFS_File PHYSFS_File;

    const char* nxFsGetError();

    PHYSFS_File* nxFsOpenRead(const char*);
    void nxFsClose(PHYSFS_File*);

    bool nxFsRead(PHYSFS_File*, PHYSFS_File*, size_t, size_t*);
    bool nxFsReadS8(PHYSFS_File*, int8_t*);
    bool nxFsReadS16(PHYSFS_File*, int16_t*);
    bool nxFsReadS32(PHYSFS_File*, int32_t*);
    bool nxFsReadS64(PHYSFS_File*, int64_t*);
    bool nxFsReadU8(PHYSFS_File*, uint8_t*);
    bool nxFsReadU16(PHYSFS_File*, uint16_t*);
    bool nxFsReadU32(PHYSFS_File*, uint32_t*);
    bool nxFsReadU64(PHYSFS_File*, uint64_t*);
    bool nxFsReadFloat(PHYSFS_File*, double*);
    bool nxFsReadDouble(PHYSFS_File*, double*);
    const char* nxFsReadString(PHYSFS_File*);
]]

------------------------------------------------------------
function InputFile:initialize(filename)
    BinaryFile.initialize(self, filename)
end

------------------------------------------------------------
function InputFile:open(filename)
    -- Close if already open
    if self:isOpen() then self:close() end

    local handle = C.nxFsOpenRead(filename)
    if handle == nil then return self:_throwError(self) end

    self._cdata = ffi.gc(handle, C.nxFsClose)
    return self
end

------------------------------------------------------------
function InputFile:read(size, asCdata)
    if self._cdata == nil then return '', 0 end

    -- If size is invalid, read all
    if type(size) ~= 'number' then size = self:size() end

    local readBytesPtr = ffi.new('size_t[1]')
    local buffer       = ffi.new('char[?]', size)

    local ok = C.nxFsRead(self._cdata, buffer, size, readBytesPtr)
    if not ok then return self:_throwError('', 0) end

    if not asCdata then buffer = ffi.string(buffer, readBytesPtr[0]) end

    return buffer, tonumber(readBytesPtr[0])
end

------------------------------------------------------------
function InputFile:readS8()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('int8_t[1]')

    local ok = C.nxFsReadS8(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readS16()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('int16_t[1]')

    local ok = C.nxFsReadS16(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readS32()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('int32_t[1]')

    local ok = C.nxFsReadS32(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readS64()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('int64_t[1]')

    local ok = C.nxFsReadS64(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return valPtr[0]
end

------------------------------------------------------------
function InputFile:readU8()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('uint8_t[1]')

    local ok = C.nxFsReadU8(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readU16()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('uint16_t[1]')

    local ok = C.nxFsReadU16(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readU32()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('uint32_t[1]')

    local ok = C.nxFsReadU32(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readU64()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('uint64_t[1]')

    local ok = C.nxFsReadU32(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return valPtr[0]
end

------------------------------------------------------------
function InputFile:readFloat()
    if self._cdata == nil then return 0 end

    -- C function does float-->string-->double to prevent the loss of precision
    -- that's due to float<->double
    local valPtr = ffi.new('double[1]')

    local ok = C.nxFsReadFloat(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readDouble()
    if self._cdata == nil then return 0 end

    local valPtr = ffi.new('double[1]')

    local ok = C.nxFsReadDouble(self._cdata, valPtr)
    if not ok then return self:_throwError(0) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readString()
    if self._cdata == nil then return '' end

    local str = C.nxFsReadString(self._cdata)
    if str == nil then self:_throwError('') end

    return ffi.string(str)
end

------------------------------------------------------------
return InputFile