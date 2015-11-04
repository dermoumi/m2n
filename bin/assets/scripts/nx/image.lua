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

ffi.cdef [[
    typedef void NxImage;
    typedef struct PHYSFS_File PHYSFS_File;

    NxImage* nxImageCreateFill(unsigned int, unsigned int, const uint8_t*);
    NxImage* nxImageCreateFromData(unsigned int, unsigned int, const uint8_t*);
    NxImage* nxImageCreateFromFile(const char* filename);
    NxImage* nxImageCreateFromMemory(const void*, size_t);
    NxImage* nxImageCreateFromHandle(PHYSFS_File*, bool close);
    bool nxImageSave(const void*, const char*);
    void nxImageRelease(NxImage*);
]]

------------------------------------------------------------
-- A class to handle Image creation and management
------------------------------------------------------------
local class = require 'nx.class'
local Image = class 'Image'

------------------------------------------------------------
function Image:initialize(filename)
    local handle = C.nxImageCreateFromFile(filename)
    if handle == nil then
        return nil, 'Cannot load image from file: ' .. filename
    end

    self._cdata = ffi.gc(handle, C.nxImageRelease)
end

------------------------------------------------------------
function Image:save(filename)
    if not self._cdata or not filename then return false end
    return C.nxImageSave(self._cdata, filename)
end

------------------------------------------------------------
return Image