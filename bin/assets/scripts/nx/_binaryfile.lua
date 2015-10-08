local ffi = require 'ffi'
local C = ffi.C

ffi.cdef[[
    typedef struct PHYSFS_File PHYSFS_File;

    const char* nxFsGetError();

    void nxFsClose(PHYSFS_File*);
    bool nxFsSize(PHYSFS_File*, size_t*);
    bool nxFsTell(PHYSFS_File*, size_t*);
    bool nxFsSeek(PHYSFS_File*, size_t);
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

return BinaryFile