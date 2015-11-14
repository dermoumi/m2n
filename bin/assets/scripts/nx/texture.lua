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

local toFilter = {
    bilinear  = 0x0,
    trilinear = 0x1,
    nearest   = 0x2,
    mask      = 0x3
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
    [16] = 0x20,
    mask = 0x3C
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
    clampcol = 0x80,
    mask     = 0xC0
}
local fromXRepeating = {
    [0x00] = 'clamp',
    [0x40] = 'wrap',
    [0x80] = 'clampcol'
}

local toYRepeating = {
    clamp    = 0x000,
    wrap     = 0x100,
    clampcol = 0x200,
    mask     = 0x300
}
local fromYRepeating = {
    [0x000] = 'clamp',
    [0x100] = 'wrap',
    [0x200] = 'clampcol'
}

local toZRepeating = {
    clamp    = 0x000,
    wrap     = 0x400,
    clampcol = 0x800,
    mask     = 0xC00
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
    typedef struct {
        uint8_t texType;
        uint32_t tex;
        uint16_t width;
        uint16_t height;
        uint16_t depth;
        uint16_t actualWidth;
        uint16_t actualHeight;
        uint32_t samplerState;
    } NxTexture;
]]

------------------------------------------------------------
-- Represents a VRAM texture
------------------------------------------------------------
local class = require 'nx.class'
local Texture = class 'nx.texture'

local Renderer = require 'nx.renderer'

------------------------------------------------------------
local function destroy(cdata)
    if cdata.tex ~= 0 then
        nxRendererDestroyTexture(cdata.tex)
    end

    C.free(cdata)
end

------------------------------------------------------------
local function validSize(size)
    if Renderer.getCapabilities('npotTexturesSupported') then
        return size
    end

    powerOfTwo = 1
    while powerOfTwo < size do
        powerOfTwo = powerOfTwo * 2
    end

    return powerOfTwo
end

------------------------------------------------------------
local function isNumber(val)
    return type(val) == 'number'
end

------------------------------------------------------------
local function isCArray(a)
    return type(a) == 'cdata' or type(a) == 'userdata'
end

------------------------------------------------------------
function Texture.static._fromCData(cdata)
    local texture = Texture:allocate()
    texture._cdata = ffi.cast('NxTexture*', cdata)
    return texture
end

------------------------------------------------------------
function Texture.static.usedMemory()
    return tonumber(nxRendererGetTextureMemory())
end

------------------------------------------------------------
function Texture:initialize()
    local handle = ffi.cast('NxTexture*', C.malloc(ffi.sizeof('NxTexture')))
    handle.texType      = 0
    handle.tex          = 0
    handle.width        = 0
    handle.height       = 0
    handle.depth        = 0
    handle.actualWidth  = 0
    handle.actualHeight = 0
    handle.samplerState = 0
    self._cdata = ffi.gc(handle, destroy)
end

------------------------------------------------------------
function Texture:release()
    if self._cdata == nil then return end
    destroy(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

------------------------------------------------------------
function Texture:create(texType, width, height, depth, hasMips, mipMap)
    -- TODO: Texture formats

    if hasMips == nil then hasMips = true end
    if mipMap == nil then mipMap = true end

    depth = depth or 1
    if width == 0 or height == 0 or depth == 0 then
        return false, 'Cannot create texture: invalid texture size'
    end

    -- Compute internal texture dimensions depending on NPOT texture support
    local actualWidth, actualHeight, actualDepth =
        validSize(width), validSize(height), validSize(depth)

    if
        texType ~= '2d' and (actualWidth ~= width or actualHeight ~= height or actualDepth ~= depth)
    then
        return false, 'Cannot create texture: 3D and Cubemap textures cannot be non-power-of-two'
    end

    texType = toTextureType[texType]
    if not texType then
        return false, 'Cannot create texture: invalid texture type'
    end

    -- Check maximum texture size
    local maxSize = Renderer.getCapabilities('maxTexSize')
    if actualWidth > maxSize or actualHeight > maxSize or depth > maxSize then
        return false, ('Cannot create texture: internal texture size is too high ' ..
            '(%ux%ux%u). Maximum is %ux%ux%u'):format(actualWidth, actualHeight, depth, maxSize,
                maxSize, maxSize)
    end

    if self._cdata.tex ~= 0 then
        C.nxRendererDestroyTexture(self._cdata.tex)
    end

    self._cdata.tex = C.nxRendererCreateTexture(texType, actualWidth, actualHeight, depth, 1,
        hasMips, mipMap, Renderer.getCapabilities('sRGBTexturesSupported'))
    if self._cdata.tex == 0 then
        return false, 'Cannot create texture'
    end

    if texType == 2 then
        for i = 0, 5 do
            C.nxRendererUploadTextureData(self._cdata.tex, i, 0, nil)
        end
    else
        C.nxRendererUploadTextureData(self._cdata.tex, 0, 0, nil)
    end

    self._cdata.texType      = texType
    self._cdata.width        = width
    self._cdata.height       = height
    self._cdata.depth        = depth
    self._cdata.actualWidth  = actualWidth
    self._cdata.actualHeight = actualHeight

    return true
end

------------------------------------------------------------
function Texture:setData(data, a, b, c, d, e, f, g, h)
    local x, y, z, width, height, depth, slice, mipLevel, wholeTexture
    if not c then
        wholeTexture, slice, mipLevel = true, a, b
    elseif self._cdata.texType == 1 then -- 3D maps
        x, y, z, width, height, depth, slice, mipLevel = a, b, c, d, e, f, g, h
    else
        x, y, z, width, height, depth, slice, mipLevel = a, b, 0, c, d, 1, e, f
    end

    slice = slice or 0
    mipLevel = mipLevel or 0

    if wholeTexture then
        C.nxRendererUploadTextureData(self._cdata.tex, slice, mipLevel, data)
    else
        C.nxRendererUploadTextureSubData(
            self._cdata.tex, slice, mipLevel, x, y, z, width, height, depth, data
        )
    end
end

------------------------------------------------------------
function Texture:bind(unit)
    if unit < Renderer.getCapabilities('maxTexUnits') then
        C.nxRendererSetTexture(unit, self._cdata.tex, self._cdata.samplerState)
        return true
    end

    return false
end

------------------------------------------------------------
function Texture:data(slice, mipLevel)
    slice    = slice or 0
    mipLevel = mipLevel or 0

    local size = nxRendererCalcTextureSize(
        1, self._cdata.internalWidth, self._cdata.internalHeight, self._cdata.depth
    )
    local buffer = ffi.new('uint8_t[?]', size)

    if not C.nxRenderer(self._cdata.tex, slice, mipLevel, buffer) then
        return nil, 'Could not recover texture data'
    end

    return buffer
end

------------------------------------------------------------
function Texture:setFilter(filter)
    filter = toFilter[filter]
    if not filter or filter == toFilter.mask then return end

    local c = self._cdata
    c.samplerState = bit.bor(bit.band(c.samplerState, bit.bnot(toFilter.mask)), filter)
end

------------------------------------------------------------
function Texture:setAnisotropyLevel(level)
    local aniso = toAniso[level]
    if not aniso or aniso == toAniso.mask then return end

    local c = self._cdata
    c.samplerState = bit.bor(bit.band(c.samplerState, bit.bnot(toAniso.mask)), aniso)
end

------------------------------------------------------------
function Texture:setRepeating(x, y, z)
    if x == 'mask' or y == 'mask' or z == 'mask' then return end

    x, y, z = toXRepeating[x], toYRepeating[y], toZRepeating[z]
    if not x or not y or not z then
        local currX, currY, currZ = self:repeating()
        x = x or toXRepeating[currX]
        y = y or toYRepeating[currY]
        z = z or toZRepeating[currZ]
    end

    local c = self._cdata
    c.samplerState = bit.bor(bit.band(c.samplerState, bit.bnot(toXRepeating.mask)), x)
    c.samplerState = bit.bor(bit.band(c.samplerState, bit.bnot(toYRepeating.mask)), y)
    c.samplerState = bit.bor(bit.band(c.samplerState, bit.bnot(toZRepeating.mask)), z)
end

------------------------------------------------------------
function Texture:setLessOrEqual(lessOrEqual)
    local c = self._cdata
    if lessOrEqual then
        c.samplerState = bit.bor(c.samplerState, 0x1000)
    else
        c.samplerState = bit.band(c.samplerState, bit.bnot(0x1000))
    end
end

------------------------------------------------------------
function Texture:filter()
    return fromFilter[bit.band(self._cdata.samplerState, toFilter.mask)] or 'bilinear'
end

------------------------------------------------------------
function Texture:anisotropyLevel()
    return fromAniso[bit.band(self._cdata.samplerState, toAniso.mask)] or 1
end

------------------------------------------------------------
function Texture:repeating()
    local x = fromXRepeating[bit.band(self._cdata.samplerState, toXRepeating.mask)] or 'clamp'
    local y = fromYRepeating[bit.band(self._cdata.samplerState, toYRepeating.mask)] or 'clamp'
    local z = fromZRepeating[bit.band(self._cdata.samplerState, toZRepeating.mask)] or 'clamp'
    return x, y, z
end

------------------------------------------------------------
function Texture:lessOrEqual()
    return bit.band(self._cdata.samplerState, 0x1000) == 0x1000
end

------------------------------------------------------------
return Texture