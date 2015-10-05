local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    typedef struct PHYSFS_File PHYSFS_File;

    const char* nxFsGetError();

    char** nxFsEnumerateFiles(const char*);
    void nxFsFreeList(char**);
    bool nxFsIsFile(const char*);
    bool nxFsIsDirectory(const char*);

    PHYSFS_File* nxFsOpenRead(const char*);
    PHYSFS_File* nxFsOpenWrite(const char*);
    bool nxFsFlush(PHYSFS_File*);
    void nxFsClose(PHYSFS_File*);
    bool nxFsSize(PHYSFS_File*, size_t*);
    bool nxFsTell(PHYSFS_File*, size_t*);
    bool nxFsSeek(PHYSFS_File*, size_t);

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

-- Shared methods between read and write files ---------------------------------
local BinaryFile = class 'BinaryFile'

function BinaryFile:isOpen()
    return self._handle ~= nil
end

function BinaryFile:close()
    if self._handle then
        C.nxFsClose(ffi.gc(self._handle, nil))
        self._handle = nil
    end
end

function BinaryFile:size()
    if self._handle then
        return nil, 'No file open'
    end

    local size = ffi.new('size_t[1]')
    if not C.nxFsSize(self._handle, size) then
        return nil, ffi.string(C.nxFsGetError())
    end

    return tonumber(size[0])
end

function BinaryFile:tell()
    if self._handle then
        return nil, 'No file open'
    end

    local position = ffi.new('size_t[1]')
    if not C.nxFsTell(self._handle, position) then
        return nil, ffi.string(C.nxFsGetError())
    end

    return tonumber(position[0])
end

function BinaryFile:seek(position)
    if self._handle then
        return nil, 'No file open'
    end

    if C.nxFsSeek(self._handle, position) then
        return true
    else
        return false, ffi.string(C.nxFsGetError())
    end
end

-- Input File ------------------------------------------------------------------
local InputFile = class('InputFile', BinaryFile)

function InputFile:initialize(filename)
    local handle = C.nxFsOpenRead(filename)
    if handle ~= nil then
        self._handle = ffi.gc(handle, C.nxFsClose)
    end
end

function InputFile:read(size)
    if not self._handle then return nil, 'No file open' end

    local readBytesPtr = ffi.new('size_t[1]')
    local buffPtr      = ffi.new('char[?]', size)

    local ok = C.nxFsRead(self._handle, buffPtr, size, readBytesPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return ffi.string(buffPtr, readBytesPtr[0]), readBytesPtr[0]
end

function InputFile:readS8()
    if not self._handle then return nil, 'No file open' end

    local valPtr = ffi.new('int8_t[1]')

    local ok = C.nxFsReadS8(self._handle, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readS16()
    if not self._handle then return nil, 'No file open' end

    local valPtr = ffi.new('int16_t[1]')

    local ok = C.nxFsReadS16(self._handle, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readS32()
    if not self._handle then return nil, 'No file open' end

    local valPtr = ffi.new('int32_t[1]')

    local ok = C.nxFsReadS32(self._handle, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readU8()
    if not self._handle then return nil, 'No file open' end

    local valPtr = ffi.new('uint8_t[1]')

    local ok = C.nxFsReadU8(self._handle, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readU16()
    if not self._handle then return nil, 'No file open' end

    local valPtr = ffi.new('uint16_t[1]')

    local ok = C.nxFsReadU16(self._handle, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readU32()
    if not self._handle then return nil, 'No file open' end

    local valPtr = ffi.new('uint32_t[1]')

    local ok = C.nxFsReadU32(self._handle, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readFloat()
    if not self._handle then return nil, 'No file open' end

    local valPtr = ffi.new('double[1]')

    local ok = C.nxFsReadFloat(self._handle, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readDouble()
    if not self._handle then return nil, 'No file open' end

    local valPtr = ffi.new('double[1]')

    local ok = C.nxFsReadDouble(self._handle, valPtr)
    if not ok then return nil, ffi.string(C.nxFsGetError()) end

    return tonumber(valPtr[0])
end

function InputFile:readString()
    if not self._handle then return nil, 'No file open' end

    local str = C.nxFsReadString(self._handle)
    if str == nil then return nil, ffi.string(C.nxFsGetError()) end

    return ffi.string(str)
end

-- Output File -----------------------------------------------------------------
local OutputFile = class('OutputFile', BinaryFile)

function OutputFile:initialize(filename)
    local handle = C.nxFsOpenWrite(filename)
    if handle ~= nil then
        self._handle = ffi.gc(handle, C.nxFsClose)
    end
end

function OutputFile:write(buffer, size)
    if not self._handle then return nil, 'No file open' end
    
    local bytesWrittenPtr = ffi.new('size_t[1]')

    local ok = C.nxFsWrite(self._handle, buffer, size, bytesWrittenPtr)
    if not ok then return false, ffi.string(C.nxFsGetError()) end

    return true
end

function OutputFile:writeS8(val)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteS8(self._handle, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeS16(val)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteS16(self._handle, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeS32(val)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteS32(self._handle, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeU8(val)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteU8(self._handle, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeU16(val)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteU16(self._handle, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeU32(val)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteU32(self._handle, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeFloat(val)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteFloat(self._handle, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeDouble(val)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteDouble(self._handle, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeString(str)
    if not self._handle then return nil, 'No file open' end

    if C.nxFsWriteString(self._handle, str) then return true end
    return false, ffi.string(C.nxFsGetError())
end

-- Main Filesystem functions ---------------------------------------------------
local function enumerateFiles(path)
    local files = C.nxFsEnumerateFiles(path)
    if files == nil then
        return nil, ffi.string(C.nxFsGetError())
    end

    local fileList = {}
    while files[#fileList] ~= nil do
        fileList[#fileList + 1] = ffi.string(files[#fileList])
    end

    C.nxFsFreeList(files)

    return fileList;
end

local function isDirectory(path)
    return C.nxFsIsDirectory(path)
end

local function isFile(path)
    return C.nxFsIsFile(path)
end

local function openRead(path)
    local file = InputFile:new(path)
    if file:isOpen() then return file end

    return nil, ffi.string(C.nxFsGetError())
end

local function openWrite(path)
    local file = OutputFile:new(path)
    if file:isOpen() then return file end

    return nil, ffi.string(C.nxFsGetError())
end

--------------------------------------------------------------------------------
return {
    enumerateFiles = enumerateFiles,
    isFile         = isFile,
    isDirectory    = isDirectory,
    openRead       = openRead,
    openWrite      = openWrite
}