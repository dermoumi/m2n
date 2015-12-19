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

local class = require 'nx.class'
local Log   = require 'nx.log'

local BinaryFile = class 'nx._binaryfile'

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
function BinaryFile.static.defaultCallback(message)
    Log.error('File I/O error: ' .. message)
end

------------------------------------------------------------
function BinaryFile:initialize(filename)
    self._errorCallback = BinaryFile.defaultCallback

    -- Attempt to open the file if given a string
    if type(filename) == 'string' then self:open(filename) end
end

------------------------------------------------------------
function BinaryFile:onError(callback)
    self._errorCallback = callback

    return self
end

------------------------------------------------------------
function BinaryFile:_throwError(retVal1, retVal2)
    self._errorCallback(ffi.string(C.nxFsGetError()))
    self:release()

    return retVal1, retVal2
end

------------------------------------------------------------
function BinaryFile:isOpen()
    return self._cdata ~= nil
end

------------------------------------------------------------
function BinaryFile:release()
    if self._cdata == nil then return end

    C.nxFsClose(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

------------------------------------------------------------
function BinaryFile:size()
    if self._cdata == nil then return 0 end

    local size = ffi.new('size_t[1]')
    if not C.nxFsSize(self._cdata, size) then return self:_throwError(0) end

    return tonumber(size[0])
end

------------------------------------------------------------
function BinaryFile:tell()
    if self._cdata == nil then return 0 end

    local position = ffi.new('size_t[1]')
    if not C.nxFsTell(self._cdata, position) then return self:_throwError(0) end

    return tonumber(position[0])
end

------------------------------------------------------------
function BinaryFile:seek(position)
    if self._cdata == nil then return self end

    if not C.nxFsSeek(self._cdata, position) then self._throwError() end

    return self
end

------------------------------------------------------------
return BinaryFile