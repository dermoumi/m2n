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
-- FFI C Declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxVectorFont NxVectorFont;
    typedef struct NxTexture NxTexture;

    NxVectorFont* nxVectorFontNew();
    void nxVectorFontRelease(NxVectorFont*);
    bool nxVectorFontOpenFromFile(NxVectorFont*, const char*);
    bool nxVectorFontOpenFromMemory(NxVectorFont*, const void*, size_t);
    bool nxVectorFontOpenFromHandle(NxVectorFont*, PHYSFS_File*);
    const char* nxVectorFontFamilyName(const NxVectorFont*);
    void nxVectorFontGlyph(const NxVectorFont*, uint32_t, uint32_t, bool, float*, uint32_t*);
    float nxVectorFontKerning(const NxVectorFont*, uint32_t, uint32_t, uint32_t);
    float nxVectorFontLineSpacing(const NxVectorFont*, uint32_t);
    float nxVectorFontUnderlinePosition(const NxVectorFont*, uint32_t);
    float nxVectorFontUnderlineThickness(const NxVectorFont*, uint32_t);
    const NxTexture* nxVectorFontTexture(const NxVectorFont*, uint32_t);
]]

------------------------------------------------------------
-- Represents a vector font object
------------------------------------------------------------
local class = require 'nx.class'
local VectorFont = class 'nx.vectorfont'

local Texture = require 'nx.texture'

------------------------------------------------------------
local function isNumber(val)
    return type(val) == 'number'
end

------------------------------------------------------------
local function isCArray(a)
    return type(a) == 'cdata' or type(a) == 'userdata'
end

------------------------------------------------------------
function VectorFont.static._fromCData(cdata)
    local font = VectorFont:allocate()
    font._cdata = ffi.cast('NxVectorFont*', cdata)
    return font
end

------------------------------------------------------------
function VectorFont:initialize()
    local handle = C.nxVectorFontNew()
    self._cdata = ffi.gc(handle, C.nxVectorFontRelease)
end

------------------------------------------------------------
function VectorFont:open(a, b)
    if isCArray(a) and isNumber(b) then
        ok = C.nxVectorFontOpenFromMemory(self._cdata, a, b)
    elseif type(a) == 'string' then
        ok = C.nxVectorFontOpenFromFile(self._cdata, a)
    elseif class.Object.isInstanceOf(a, require('nx.inputfile')) then
        ok = C.nxVectorFontOpenFromHandle(self._cdata, a._cdata)
    else
        return false, 'Invalid parameters'
    end

    if not ok then
        return 'Cannot open font'
    end

    return true
end

------------------------------------------------------------
function VectorFont:familyName()
    return ffi.string(C.nxVectorFontFamilyName(self._cdata))
end

------------------------------------------------------------
function VectorFont:glyph(codePoint, charSize, bold)
    local fValues = ffi.new('float[5]')
    local uValues = ffi.new('uint32_t[4]')
    C.nxVectorFontGlyph(self._cdata, codePoint, charSize, not not bold, fValues, uValues)

    return fValues[0], fValues[1], fValues[2], fValues[3], fValues[4], uValues[0], uValues[1],
        uValues[2], uValues[4]
end

------------------------------------------------------------
function VectorFont:kerning(first, second, charSize)
    return C.nxVectorFontKerning(self._cdata, first, second, charSize)
end

------------------------------------------------------------
function VectorFont:lineSpacing(charSize)
    return C.nxVectorFontLineSpacing(self._cdata, charSize)
end

------------------------------------------------------------
function VectorFont:underlinePosition(charSize)
    return C.nxVectorFontUnderlinePosition(self._cdata, charSize)
end

------------------------------------------------------------
function VectorFont:underlineThickness(charSize)
    return C.nxVectorFontUnderlineThickness(self._data, charSize)
end

------------------------------------------------------------
function VectorFont:texture(charSize)
    return Texture._fromCData(C.nxVectorFontTexture(self._cdata, charSize))
end

------------------------------------------------------------
return VectorFont