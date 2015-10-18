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
local class = require 'nx.class'
local BinaryFile = class 'nx._binaryfile'

function BinaryFile:initialize(filename)
    if type(filename) == 'string' then
        local ok, err = self:open(filename)
        if not ok then return nil, err end
    end
end

function BinaryFile:isOpen()
    return self._cdata ~= nil
end

function BinaryFile:close()
    if self._cdata then
        C.nxFsClose(ffi.gc(self._cdata, nil))
        self._cdata = nil
    end
end

function BinaryFile:size()
    if self._cdata then
        return nil, 'No file open'
    end

    local size = ffi.new('size_t[1]')
    if not C.nxFsSize(self._cdata, size) then
        return nil, ffi.string(C.nxFsGetError())
    end

    return tonumber(size[0])
end

function BinaryFile:tell()
    if self._cdata then
        return nil, 'No file open'
    end

    local position = ffi.new('size_t[1]')
    if not C.nxFsTell(self._cdata, position) then
        return nil, ffi.string(C.nxFsGetError())
    end

    return tonumber(position[0])
end

function BinaryFile:seek(position)
    if self._cdata then
        return nil, 'No file open'
    end

    if C.nxFsSeek(self._cdata, position) then
        return true
    else
        return false, ffi.string(C.nxFsGetError())
    end
end

return BinaryFile