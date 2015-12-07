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
-- Constants
------------------------------------------------------------
local toTextureType = {
    ['2d'] = 0,
    ['3d'] = 1,
    ['cube'] = 2
}
local fromTextureType = {
    [0] = '2d',
    '3d',
    'cube'
}

local toTextureFormat = {
    -- TODO
}
local fromTextureFormat = {
    -- TODO
}

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
    clamp    = 0x00,
    wrap     = 0x40,
    clampcol = 0x80
}
local fromXRepeating = {
    [0x00] = 'clamp',
    [0x40] = 'wrap',
    [0x80] = 'clampcol'
}

local toYRepeating = {
    clamp    = 0x000,
    wrap     = 0x100,
    clampcol = 0x200
}
local fromYRepeating = {
    [0x000] = 'clamp',
    [0x100] = 'wrap',
    [0x200] = 'clampcol'
}

local toZRepeating = {
    clamp    = 0x000,
    wrap     = 0x400,
    clampcol = 0x800
}
local fromZRepeating = {
    [0x000] = 'clamp',
    [0x400] = 'wrap',
    [0x800] = 'clampcol'
}

------------------------------------------------------------
-- FFI C Declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxTexture NxTexture;

    NxTexture* nxTextureNew();
    void nxTextureRelease(NxTexture*);
    uint8_t nxTextureCreate(NxTexture*, uint8_t, uint8_t, uint16_t, uint16_t, uint16_t, bool, bool,
        bool);
    void nxTextureSetData(NxTexture*, const void*, int32_t, int32_t, int32_t, int32_t, int32_t,
        int32_t, uint8_t, uint8_t);
    bool nxTextureData(const NxTexture*, void*, uint8_t, uint8_t);
    void nxTextureSize(const NxTexture*, uint16_t*);
    uint32_t nxTextureBufferSize(const NxTexture*);
    void nxTextureSetFilter(NxTexture*, uint32_t);
    void nxTextureSetAnisotropyLevel(NxTexture*, uint32_t);
    void nxTextureSetRepeatingX(NxTexture*, uint32_t);
    void nxTextureSetRepeatingY(NxTexture*, uint32_t);
    void nxTextureSetRepeatingZ(NxTexture*, uint32_t);
    void nxTextureSetLessOrEqual(NxTexture*, bool);
    uint32_t nxTextureFilter(const NxTexture*);
    uint32_t nxTextureAnisotropyLevel(const NxTexture*);
    void nxTextureRepeating(const NxTexture*, uint32_t*);
    bool nxTextureLessOrEqual(const NxTexture*);
    bool nxTextureFlipCoords(const NxTexture*);
    uint8_t nxTextureType(const NxTexture*);
    uint8_t nxTextureFormat(const NxTexture*);
    uint32_t nxTextureUsedMemory();
    void nxTextureBind(const NxTexture*, uint8_t);
]]

------------------------------------------------------------
-- Represents a VRAM texture
------------------------------------------------------------
local class = require 'nx.class'
local Texture = class 'nx.texture'

------------------------------------------------------------
function Texture.static._fromCData(cdata)
    local texture = Texture:allocate()
    texture._cdata = ffi.cast('NxTexture*', cdata)
    return texture
end

------------------------------------------------------------
function Texture.static.usedMemory()
    return C.nxTextureUsedMemory();
end

------------------------------------------------------------
function Texture.static.bind(texture, slot)
    if texture then texture = texture._cdata end
    C.nxTextureBind(texture, slot)
    return Texture
end

------------------------------------------------------------
function Texture:initialize(texType, width, height, depth, hasMips, mipMap)
    local handle = C.nxTextureNew()
    self._cdata = ffi.gc(handle, C.nxTextureRelease)

    if texType and width and height then
        ok, err = self:create(texType, width, height, depth, hasMips, mipMap)
        if not ok then return nil, err end
    end
end

------------------------------------------------------------
function Texture:release()
    if self._cdata == nil then return end
    C.nxTextureRelease(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

------------------------------------------------------------
function Texture:create(texType, width, height, depth, hasMips, mipMap)
    -- TODO: Texture formats
    local Renderer = require('nx.renderer')

    if hasMips == nil then hasMips = true end
    if mipMap == nil then mipMap = true end
    depth = depth or 1

    texType = toTextureType[texType]
    if not texType then
        return false, 'Cannot create texture: invalid texture type'
    end

    local status = C.nxTextureCreate(self._cdata, texType, 1, width, height, depth, hasMips,
        mipMap, Renderer.getCapabilities('sRGBTexturesSupported'))

    if status == 1 then
        return false, 'Cannot create texture: invalid texture size'
    elseif status == 2 then
        local max = Renderer.getCapabilities('maxTexSize')
        return false, ('Cannot create texture: internal texture size is too high ' ..
            '(%ux%ux%u). Maximum is %ux%ux%u'):format(width, height, depth, max, max, max)
    elseif status == 3 then
        return false, 'Cannot create texture'
    end

    return true
end

------------------------------------------------------------
function Texture:setData(data, a, b, c, d, e, f, g, h)
    local x, y, z, width, height, depth, slice, mipLevel
    if not c then
        x, y, z, width, height, depth, slice, mipLevel = -1, -1, -1, -1, -1, -1, a, b
    elseif C.nxTextureType(self._cdata) == 1 then -- 3D maps
        x, y, z, width, height, depth, slice, mipLevel = a, b, c, d, e, f, g, h
    else
        x, y, z, width, height, depth, slice, mipLevel = a, b, 0, c, d, 1, e, f
    end

    if class.Object.isInstanceOf(data, require('nx.image')) then
        data = data:data()
    end

    slice = slice or 0
    mipLevel = mipLevel or 0

    C.nxTextureSetData(self._cdata, data, x, y, z, width, height, depth, slice, mipLevel)
    return self
end

------------------------------------------------------------
function Texture:bind(slot)
    if not self._cdata then return end
    C.nxTextureBind(self._cdata, slot or 0)

    return self
end

------------------------------------------------------------
function Texture:data(slice, mipLevel)
    slice    = slice or 0
    mipLevel = mipLevel or 0

    local buffer = ffi.new('uint8_t[?]', C.nxTextureBufferSize(self._cdata))

    if not C.nxTextureData(self._cdata, buffer, slice, mipLevel) then
        return nil, 'Could not recover texture data'
    end

    return buffer
end

------------------------------------------------------------
function Texture:size()
    -- if self._cdata == nil then return 0, 0 end
    
    local sizePtr = ffi.new('uint16_t[3]')
    C.nxTextureSize(self._cdata, sizePtr)
    return sizePtr[0], sizePtr[1], sizePtr[2]
end

------------------------------------------------------------
function Texture:setFilter(filter)
    filter = toFilter[filter]
    if not filter then return end

    C.nxTextureSetFilter(self._cdata, filter)
    return self
end

------------------------------------------------------------
function Texture:setAnisotropyLevel(level)
    local aniso = toAniso[level]
    if not aniso then return end

    C.nxTextureSetAnisotropyLevel(self._cdata, aniso)
    return self
end

------------------------------------------------------------
function Texture:setRepeating(x, y, z)
    x, y, z = toXRepeating[x], toYRepeating[y], toZRepeating[z]

    if x then C.nxTextureSetRepeatingX(self._cdata, x) end
    if y then C.nxTextureSetRepeatingY(self._cdata, y) end
    if z then C.nxTextureSetRepeatingZ(self._cdata, z) end

    return self
end

------------------------------------------------------------
function Texture:setLessOrEqual(lessOrEqual)
    C.nxTextureSetLessOrEqual(self._cdata, not not lessOrEqual)

    return self
end

------------------------------------------------------------
function Texture:filter()
    return fromFilter[C.nxTextureFilter(self._cdata)] or 'bilinear'
end

------------------------------------------------------------
function Texture:anisotropyLevel()
    return fromAniso[C.nxTextureAnisotropyLevel(self._cdata)] or 1
end

------------------------------------------------------------
function Texture:repeating()
    local repeatingPtr = ffi.new('uint32_t[3]')
    C.nxTextureRepeating(self._cdata, repeatingPtr)

    local x = fromXRepeating[repeatingPtr[0]] or 'clamp'
    local y = fromYRepeating[repeatingPtr[1]] or 'clamp'
    local z = fromZRepeating[repeatingPtr[2]] or 'clamp'

    return x, y, z
end

------------------------------------------------------------
function Texture:lessOrEqual()
    return C.nxTextureLessOrEqual(self._cdata)
end

------------------------------------------------------------
function Texture:flipCoords()
    return C.nxTextureFlipCoords(self._cdata)
end

------------------------------------------------------------
function Texture:texType()
    return fromTextureType[C.nxTextureType(self._cdata)]
end

------------------------------------------------------------
function Texture:texFormat()
    return fromTextureFormat[C.nxTextureFormat(self._cdata)]
end

------------------------------------------------------------
return Texture