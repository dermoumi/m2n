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
-- Represents a vector font object
------------------------------------------------------------
local Config     = require 'nx.config'
local Font       = require 'nx.font'
local VectorFont = Font:subclass('nx.vectorfont')

------------------------------------------------------------
-- FFI C Declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    NxFont* nxVectorFontNew();
    bool nxVectorFontOpenFromFile(NxFont*, const char*);
    bool nxVectorFontOpenFromMemory(NxFont*, const void*, size_t);
    bool nxVectorFontOpenFromHandle(NxFont*, PHYSFS_File*);
    const char* nxVectorFontFamilyName(const NxFont*);
]]

------------------------------------------------------------
function VectorFont:initialize(a, b)
    local handle = C.nxVectorFontNew()
    self._cdata = ffi.gc(handle, C.nxFontRelease)

    -- If given an argument, pass it to :open()
    if a then self:open(a, b) end
end

------------------------------------------------------------
function VectorFont:open(a, b)
    local ok

    if type(b) == 'number' then
        ok = C.nxVectorFontOpenFromMemory(self._cdata, a, b)
    elseif type(a) == 'string' then
        ok = C.nxVectorFontOpenFromFile(self._cdata, a)
    else
        ok = C.nxVectorFontOpenFromHandle(self._cdata, a._cdata)
    end

    if not ok then
        C.nxVectorFontOpenFromFile(self._cdata, Config.defaultVectorFont)
    end

    return self
end

------------------------------------------------------------
function VectorFont:familyName()
    return ffi.string(C.nxVectorFontFamilyName(self._cdata))
end

------------------------------------------------------------
return VectorFont