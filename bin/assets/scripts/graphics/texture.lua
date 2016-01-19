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

local bit      = require 'bit'
local class    = require 'class'
local Config   = require 'config'
local Log      = require 'util.log'
local Graphics = require 'graphics'
local Image    = require 'graphics.image'

local Texture = class 'graphics.texture'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxTexture NxTexture;

    NxTexture* nxTextureNew();
    void nxTextureRelease(NxTexture*);
    bool nxTextureCreate(NxTexture*, uint8_t, uint8_t, uint16_t, uint16_t, bool, bool,
        bool);
    void nxTextureSetData(NxTexture*, const void*, uint8_t, uint8_t);
    void nxTextureSetSubData(NxTexture*, const void*, uint16_t, uint16_t, uint16_t, uint16_t,
        uint8_t, uint8_t);
    bool nxTextureData(const NxTexture*, void*, uint8_t, uint8_t);
    void nxTextureSize(const NxTexture*, uint16_t*);
    uint32_t nxTextureBufferSize(const NxTexture*);
    void nxTextureSetFilter(NxTexture*, uint32_t);
    void nxTextureSetAnisotropyLevel(NxTexture*, uint32_t);
    void nxTextureSetRepeating(NxTexture*, uint32_t);
    void nxTextureSetLessOrEqual(NxTexture*, bool);
    uint32_t nxTextureFilter(const NxTexture*);
    uint32_t nxTextureAnisotropyLevel(const NxTexture*);
    uint32_t nxTextureRepeating(const NxTexture*);
    bool nxTextureLessOrEqual(const NxTexture*);
    bool nxTextureFlipCoords(const NxTexture*);
    uint8_t nxTextureType(const NxTexture*);
    uint8_t nxTextureFormat(const NxTexture*);
    uint32_t nxTextureUsedMemory();
    void nxTextureBind(const NxTexture*, uint8_t);
]]

local toTextureType = {
    ['2d'] = 0,
    ['cube'] = 1
}
local fromTextureType = {
    [0] = '2d',
    'cube'
}

local fromTextureFormat = {
    [0] = 'unknown',
    
    'rgba8',
    'dxt1',
    'dxt3',
    'dxt5',
    'rgba16f',
    'rgba32f',

    'pvrtci2bpp',
    'pvrtcia2bpp',
    'pvrtci4bpp',
    'pvrtcia4bpp',
    'etc1',

    'depth'
}
local toTextureFormat = {}
for i, v in ipairs(fromTextureFormat) do
    toTextureFormat[v] = i
end

local toFilter = {
    bilinear  = 0x0,
    trilinear = 0x1,
    nearest   = 0x2
}
local fromFilter = {
    [0x0] = 'bilinear',
    [0x1] = 'trilinear',
    [0x2] = 'nearest'
}

local toAniso = {
    [1]  = 0x00,
    [2]  = 0x04,
    [4]  = 0x08,
    [8]  = 0x10,
    [16] = 0x20
}
local fromAniso = {
    [0x00] = 1,
    [0x04] = 2,
    [0x08] = 4,
    [0x10] = 8,
    [0x20] = 16
}

local toXRepeating = {
    clampx   = 0x00,
    wrapx    = 0x40,
    stretchx = 0x80
}
local fromXRepeating = {
    [0x00] = 'clampx',
    [0x40] = 'wrapx',
    [0x80] = 'stretchx'
}

local toYRepeating = {
    clampy   = 0x000,
    wrapy    = 0x100,
    stretchy = 0x200
}
local fromYRepeating = {
    [0x000] = 'clampy',
    [0x100] = 'wrapy',
    [0x200] = 'stretchy'
}

local toRepeating = {
    clamp   = 0,
    wrap    = 320,
    stretch = 640
}
local fromRepeating = {
    [0]   = 'clamp',
    [320] = 'wrap',
    [640] = 'stretch'
}

function Texture.static.usedMemory()
    return C.nxTextureUsedMemory();
end

function Texture.static.bind(texture, slot)
    C.nxTextureBind(texture and texture._cdata, slot)

    return Texture
end

function Texture:initialize(texType, width, height, hasMips, mipMap)
    self._cdata = ffi.gc(C.nxTextureNew(), C.nxTextureRelease)

    if texType and width and height then
        self:create(texType, width, height, hasMips, mipMap)
    end
end

function Texture:release()
    if self.__wk_status == 'failed' then return end

    C.nxTextureRelease(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

function Texture:create(texType, width, height, hasMips, mipMap)
    if hasMips == nil then hasMips = true end
    if mipMap == nil  then mipMap = true end

    texType = toTextureType[texType]
    if texType then
        local status = C.nxTextureCreate(
            self._cdata, texType, toTextureFormat[Config.textureFormat] or 1, width, height,
            hasMips, mipMap, Graphics.getCapabilities('sRGBTexturesSupported')
        )
    else
        Log.warning('Invalid texture type')
    end

    return self
end

function Texture:setData(data, slice, level)
    if self.__wk_status ~= 'failed' then

        if class.Object.isInstanceOf(data, Image) then
            data = data:data()
        end

        C.nxTextureSetData(self._cdata, data, slice or 0, level or 0)
    end

    return self
end

function Texture:setSubData(data, x, y, z, width, height, slice, level)
    if self.__wk_status ~= 'failed' then
        if class.Object.isInstanceOf(data, Image) then data = data:data() end

        C.nxTextureSetSubData(self._cdata, data, x, y, width, height, slice or 0, level or 0)
    end

    return self
end

function Texture:bind(slot)
    C.nxTextureBind(self._cdata, slot or 0)

    return self
end

function Texture:data(slice, level)
    if self.__wk_status == 'failed' then return nil end

    local buffer = ffi.new('uint8_t[?]', C.nxTextureBufferSize(self._cdata))

    if C.nxTextureData(self._cdata, buffer, slice or 0, level or 0) then
        return buffer
    else
        return nil
    end
end

function Texture:size()
    if self.__wk_status == 'failed' then return 0, 0 end
    
    local sizePtr = ffi.new('uint16_t[2]')
    C.nxTextureSize(self._cdata, sizePtr)
    return sizePtr[0], sizePtr[1]
end

function Texture:setFilter(filter)
    if self.__wk_status ~= 'failed' then
        C.nxTextureSetFilter(self._cdata, toFilter[filter] or 0)
    end

    return self
end

function Texture:setAnisotropyLevel(level)
    if self.__wk_status ~= 'failed' then
        C.nxTextureSetAnisotropyLevel(self._cdata, toAniso[level] or 0)
    end

    return self
end

function Texture:setRepeating(x, y)
    if self.__wk_status ~= 'failed' then
        C.nxTextureSetRepeating(
            self._cdata, y and bit.bor(toXRepeating[x], toYRepeating[y]) or toRepeating[x]
        )
    end

    return self
end

function Texture:setLessOrEqual(lessOrEqual)
    if self.__wk_status ~= 'failed' then
        C.nxTextureSetLessOrEqual(self._cdata, not not lessOrEqual)
    end

    return self
end

function Texture:filter()
    return fromFilter[C.nxTextureFilter(self._cdata)] or 'bilinear'
end

function Texture:anisotropyLevel()
    return fromAniso[C.nxTextureAnisotropyLevel(self._cdata)] or 1
end

function Texture:repeating()
    local repeating = C.nxTextureRepeating(self._cdata)
    return fromXRepeating[bit.band(repeating, 192)], fromYRepeating[bit.band(repeating, 768)]
end

function Texture:lessOrEqual()
    return C.nxTextureLessOrEqual(self._cdata)
end

function Texture:flipCoords()
    return C.nxTextureFlipCoords(self._cdata)
end

function Texture:type()
    return fromTextureType[C.nxTextureType(self._cdata)]
end

function Texture:format()
    return fromTextureFormat[C.nxTextureFormat(self._cdata)]
end

return Texture