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

-- Output File -----------------------------------------------------------------
local class = require 'nx.class'
local BinaryFile = require 'nx._binaryfile'
local OutputFile = class('nx.outputfile', BinaryFile)

function OutputFile.static._fromCData(data)
    local file = OutputFile:new()
    file._cdata = ffi.cast('PHYSFS_File*', data)
    return file
end

function OutputFile:initialize(filename)
    return BinaryFile.initialize(self, filename)
end

function OutputFile:open(filename)
    if self:isOpen() then
        self:close()
    end

    local handle = C.nxFsOpenWrite(filename)
    if handle ~= nil then
        self._cdata = ffi.gc(handle, C.nxFsClose)
        return true
    else
        return false, ffi.string(C.nxFsGetError())
    end
end

function OutputFile:write(buffer, size)
    if not self._cdata then return nil, 'No file open' end
    
    local bytesWrittenPtr = ffi.new('size_t[1]')

    local ok = C.nxFsWrite(self._cdata, buffer, size, bytesWrittenPtr)
    if not ok then return false, ffi.string(C.nxFsGetError()) end

    return true
end

function OutputFile:writeS8(val)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteS8(self._cdata, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeS16(val)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteS16(self._cdata, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeS32(val)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteS32(self._cdata, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeU8(val)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteU8(self._cdata, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeU16(val)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteU16(self._cdata, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeU32(val)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteU32(self._cdata, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeFloat(val)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteFloat(self._cdata, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeDouble(val)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteDouble(self._cdata, val) then return true end
    return false, ffi.string(C.nxFsGetError())
end

function OutputFile:writeString(str)
    if not self._cdata then return nil, 'No file open' end

    if C.nxFsWriteString(self._cdata, str) then return true end
    return false, ffi.string(C.nxFsGetError())
end

return OutputFile