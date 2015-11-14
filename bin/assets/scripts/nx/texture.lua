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
function Texture:setData(data, slice, mipLevel)
    slice    = slice or 0
    mipLevel = mipLevel or 0

    C.nxRendererUploadTextureData(self._cdata.tex, slice, mipLevel, data)
end

------------------------------------------------------------
function Texture:setSubdata(data, a, b, c, d, e, f, g, h)
    local x, y, z, width, height, depth, slice, mipLevel
    if self._cdata.texType == 1 then -- 3D maps
        x, y, z, width, height, depth, slice, mipLevel = a, b, c, d, e, f, g, h
    else
        x, y, z, width, height, depth, slice, mipLevel = a, b, 0, c, d, 1, e, f
    end

    slice = slice or 0
    mipLevel = mipLevel or 0

    C.nxRendererUploadTextureSubData(
        self._cdata.tex, slice, mipLevel, x, y, z, width, height, depth, data
    )
end

------------------------------------------------------------
function Texture:bind(unit)
    if unit < Renderer.getCapabilities('maxTexUnits') then
        C.nxRendererSetTexture(unit, self._cdata.tex, 0)
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
return Texture