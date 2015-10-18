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

-- Input File ------------------------------------------------------------------
local class = require 'nx.class'
local BinaryFile = require 'nx._binaryfile'
local InputFile = class('nx.inputfile', BinaryFile)

function InputFile.static._fromCData(data)
    local file = InputFile:new()
    file._cdata = ffi.cast('PHYSFS_File*', data)
    return file
end

function InputFile:initialize(filename)
    return BinaryFile.initialize(self, filename)
end

function InputFile:open(filename)
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

function InputFile:read(size)
    if not self._cdata then return nil, 'No file open' end

    local readBytesPtr = ffi.new('size_t[1]')
    local buffPtr      = ffi.new('char[?]', size)

    local ok = C.nxFsRead(self._cdata, buffPtr, size, readBytesPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return ffi.string(buffPtr, readBytesPtr[0]), readBytesPtr[0]
end

function InputFile:readS8()
    if not self._cdata then return nil, 'No file open' end

    local valPtr = ffi.new('int8_t[1]')

    local ok = C.nxFsReadS8(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readS16()
    if not self._cdata then return nil, 'No file open' end

    local valPtr = ffi.new('int16_t[1]')

    local ok = C.nxFsReadS16(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readS32()
    if not self._cdata then return nil, 'No file open' end

    local valPtr = ffi.new('int32_t[1]')

    local ok = C.nxFsReadS32(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readU8()
    if not self._cdata then return nil, 'No file open' end

    local valPtr = ffi.new('uint8_t[1]')

    local ok = C.nxFsReadU8(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readU16()
    if not self._cdata then return nil, 'No file open' end

    local valPtr = ffi.new('uint16_t[1]')

    local ok = C.nxFsReadU16(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readU32()
    if not self._cdata then return nil, 'No file open' end

    local valPtr = ffi.new('uint32_t[1]')

    local ok = C.nxFsReadU32(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readFloat()
    if not self._cdata then return nil, 'No file open' end

    local valPtr = ffi.new('double[1]')

    local ok = C.nxFsReadFloat(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readDouble()
    if not self._cdata then return nil, 'No file open' end

    local valPtr = ffi.new('double[1]')

    local ok = C.nxFsReadDouble(self._cdata, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readString()
    if not self._cdata then return nil, 'No file open' end

    local str = C.nxFsReadString(self._cdata)
    if str == nil then return nil, ffi.string(C.nxFsGetError()) end

    return ffi.string(str)
end

return InputFile