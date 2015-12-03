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
    typedef struct NxFont NxFont;
    typedef struct NxTexture NxTexture;

    void nxFontRelease(NxFont*);
    void nxFontGlyph(const NxFont*, uint32_t, uint32_t, bool, float*, uint32_t*);
    float nxFontKerning(const NxFont*, uint32_t, uint32_t, uint32_t);
    float nxFontLineSpacing(const NxFont*, uint32_t);
    float nxFontUnderlinePosition(const NxFont*, uint32_t);
    float nxFontUnderlineThickness(const NxFont*, uint32_t);
    const NxTexture* nxFontTexture(const NxFont*, uint32_t, uint32_t);
]]

------------------------------------------------------------
-- Represents a vector font object
------------------------------------------------------------
local class = require 'nx.class'
local Font = class 'nx.font'

local Texture = require 'nx.texture'

------------------------------------------------------------
function Font.static._fromCData(cdata)
    local font = Font:allocate()
    font._cdata = ffi.cast('NxFont*', cdata)
    return font
end

------------------------------------------------------------
function Font:release()
    if not self._cdata then return end
    C.nxFontRelease(self._cdata)
end

------------------------------------------------------------
function Font:glyph(codePoint, charSize, bold)
    local fValues = ffi.new('float[5]')
    local uValues = ffi.new('uint32_t[4]')
    C.nxFontGlyph(self._cdata, codePoint, charSize, not not bold, fValues, uValues)

    return fValues[0], fValues[1], fValues[2], fValues[3], fValues[4], uValues[0], uValues[1],
        uValues[2], uValues[4]
end

------------------------------------------------------------
function Font:kerning(first, second, charSize)
    return C.nxFontKerning(self._cdata, first, second, charSize)
end

------------------------------------------------------------
function Font:lineSpacing(charSize)
    return C.nxFontLineSpacing(self._cdata, charSize)
end

------------------------------------------------------------
function Font:underlinePosition(charSize)
    return C.nxFontUnderlinePosition(self._cdata, charSize)
end

------------------------------------------------------------
function Font:underlineThickness(charSize)
    return C.nxFontUnderlineThickness(self._data, charSize)
end

------------------------------------------------------------
function Font:texture(charSize, index)
    return Texture._fromCData(C.nxFontTexture(self._cdata, charSize, index or 0))
end

------------------------------------------------------------
return Font