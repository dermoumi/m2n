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

    void nxFsClose(PHYSFS_File*);
    bool nxFsSize(PHYSFS_File*, size_t*);
    bool nxFsTell(PHYSFS_File*, size_t*);
    bool nxFsSeek(PHYSFS_File*, size_t);
]]

------------------------------------------------------------
-- Base class for InputFile and OutputFile
------------------------------------------------------------
local class = require 'nx.class'
local BinaryFile = class 'nx._binaryfile'

------------------------------------------------------------
function BinaryFile:initialize(filename)
    -- Attempt to open the file if the argument is valid
    if type(filename) == 'string' and filename == '' then
        -- Call the subclass' open() method
        local ok, err = self:open(filename)
        if not ok then return nil, err end
    end
end

------------------------------------------------------------
function BinaryFile:isOpen()
    return self._cdata ~= nil
end

------------------------------------------------------------
function BinaryFile:close()
    -- Make sure the file is open
    if self._cdata then
        -- Make the cdata unmanaged before setting it to nil
        C.nxFsClose(ffi.gc(self._cdata, nil))
        self._cdata = nil
    end
end

------------------------------------------------------------
function BinaryFile:size()
    -- Make sure the file is open
    if not self._cdata then
        return 0, 'No file open'
    end

    local size = ffi.new('size_t[1]')
    if not C.nxFsSize(self._cdata, size) then
        return 0, ffi.string(C.nxFsGetError())
    end

    return tonumber(size[0])
end

------------------------------------------------------------
function BinaryFile:tell()
    -- Make sure the file is open
    if not self._cdata then
        return 0, 'No file open'
    end

    local position = ffi.new('size_t[1]')
    if not C.nxFsTell(self._cdata, position) then
        return 0, ffi.string(C.nxFsGetError())
    end

    return tonumber(position[0])
end

------------------------------------------------------------
function BinaryFile:seek(position)
    -- Make sure the file is open
    if not self._cdata then
        return false, 'No file open'
    end

    -- Make sure the argument is valid
    if type(position) ~= 'number' then
        return false, 'Invalid argument'
    end

    if C.nxFsSeek(self._cdata, position) then
        return true
    else
        return false, ffi.string(C.nxFsGetError())
    end
end

------------------------------------------------------------
return BinaryFile