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
-- A class to open files for reading
------------------------------------------------------------
local class = require 'nx.class'
local BinaryFile = require 'nx._binaryfile'
local InputFile = class('nx.inputfile', BinaryFile)

------------------------------------------------------------
function InputFile.static._fromCData(data)
    local file = InputFile:new()
    file._cdata = ffi.cast('PHYSFS_File*', data)
    return file
end

------------------------------------------------------------
function InputFile:initialize(filename)
    return BinaryFile.initialize(self, filename)
end

------------------------------------------------------------
function InputFile:open(filename)
    -- Close if already open
    if self:isOpen() then
        self:close()
    end

    local handle = C.nxFsOpenRead(filename)
    if handle ~= nil then
        self._cdata = ffi.gc(handle, C.nxFsClose)
        return true
    else
        return false, ffi.string(C.nxFsGetError())
    end
end

------------------------------------------------------------
function InputFile:read(size)
    -- If size is invalid, read all
    if type(size) ~= 'number' then
        size = self:size()
    end

    local readBytesPtr = ffi.new('size_t[1]')
    local buffPtr      = ffi.new('char[?]', size)

    local ok = C.nxFsRead(self._cdata, buffPtr, size, readBytesPtr)
    if not ok then return '', ffi.string(C.nxFsGetError()) end

    return ffi.string(buffPtr, readBytesPtr[0]), tonumber(readBytesPtr[0])
end

------------------------------------------------------------
function InputFile:readS8()
    local valPtr = ffi.new('int8_t[1]')

    local ok = C.nxFsReadS8(self._cdata, valPtr)
    if not ok then return 0, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readS16()
    local valPtr = ffi.new('int16_t[1]')

    local ok = C.nxFsReadS16(self._cdata, valPtr)
    if not ok then return 0, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readS32()
    local valPtr = ffi.new('int32_t[1]')

    local ok = C.nxFsReadS32(self._cdata, valPtr)
    if not ok then  return 0, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readU8()
    local valPtr = ffi.new('uint8_t[1]')

    local ok = C.nxFsReadU8(self._cdata, valPtr)
    if not ok then return 0, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readU16()
    local valPtr = ffi.new('uint16_t[1]')

    local ok = C.nxFsReadU16(self._cdata, valPtr)
    if not ok then return 0, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readU32()
    local valPtr = ffi.new('uint32_t[1]')

    local ok = C.nxFsReadU32(self._cdata, valPtr)
    if not ok then return 0, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readFloat()
    local valPtr = ffi.new('double[1]')

    local ok = C.nxFsReadFloat(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readDouble()
    local valPtr = ffi.new('double[1]')

    local ok = C.nxFsReadDouble(self._cdata, valPtr)
    if not ok then return 0, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

------------------------------------------------------------
function InputFile:readString()
    local str = C.nxFsReadString(self._cdata)
    if str == nil then return '', ffi.string(C.nxFsGetError()) end

    return ffi.string(str)
end

------------------------------------------------------------
return InputFile