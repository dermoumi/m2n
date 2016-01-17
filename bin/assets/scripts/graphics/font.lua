--[[
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
--]]

local class   = require 'class'
local Texture = require 'graphics.texture'

local Font = class 'graphics.font'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxFont NxFont;
    typedef struct NxTexture NxTexture;

    void nxFontRelease(NxFont*);
    void nxFontGlyph(const NxFont*, uint32_t, uint32_t, bool, double*);
    float nxFontKerning(const NxFont*, uint32_t, uint32_t, uint32_t);
    float nxFontLineSpacing(const NxFont*, uint32_t);
    float nxFontUnderlinePosition(const NxFont*, uint32_t);
    float nxFontUnderlineThickness(const NxFont*, uint32_t);
    const NxTexture* nxFontTexture(const NxFont*, uint32_t, uint32_t);
]]

function Font:release()
    if not self._cdata then return end
    C.nxFontRelease(self._cdata)
    self._cdata = nil
end

function Font:glyph(codePoint, charSize, bold)
    if self._cdata == nil then return 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false end

    local values = ffi.new('double[11]')
    C.nxFontGlyph(self._cdata, codePoint, charSize, not not bold, values)

    return values[0], -- advance
        values[1], values[2], values[3], values[4], -- glyph left, top, width, height
        values[5], values[6], values[7], values[8], -- texCoords left, top, width, height
        values[9], values[10] ~= 0 -- page, isValid
end

function Font:kerning(first, second, charSize)
    if self._cdata == nil then return 0 end
    return C.nxFontKerning(self._cdata, first, second, charSize)
end

function Font:lineSpacing(charSize)
    if self._cdata == nil then return 0 end
    return C.nxFontLineSpacing(self._cdata, charSize)
end

function Font:underlinePosition(charSize)
    if self._cdata == nil then return 0 end
    return C.nxFontUnderlinePosition(self._cdata, charSize)
end

function Font:underlineThickness(charSize)
    if self._cdata == nil then return 0 end
    return C.nxFontUnderlineThickness(self._data, charSize)
end

function Font:texture(charSize, index)
    if self._cdata == nil then return Texture:new() end

    if self._textures and self._textures[charSize] and self._textures[charSize][index] then
        return self._textures[charSize][index]
    end

    if not self._textures then self._textures = {} end
    if not self._textures[charSize] then self._textures[charSize] = {} end

    local texture = Texture:allocate()
    texture._cdata = C.nxFontTexture(self._cdata, charSize, index or 0)
    self._textures[charSize][index] = texture

    return texture
end

return Font