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
    typedef struct NxImage NxImage;
    typedef struct PHYSFS_File PHYSFS_File;

    NxImage* nxImageNew();
    void nxImageRelease(NxImage*);
    void nxImageCreateFill(NxImage*, uint32_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void nxImageCreateFromData(NxImage*, uint32_t, uint32_t, const uint8_t*);
    bool nxImageOpenFromFile(NxImage*, const char*);
    bool nxImageOpenFromMemory(NxImage*, const void*, size_t);
    bool nxImageOpenFromHandle(NxImage*, PHYSFS_File*, bool);
    bool nxImageSave(const NxImage*, const char*);
    void nxImageGetSize(const NxImage*, uint32_t*);
    void nxImageColorMask(NxImage*, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void nxImageCopyPixels(NxImage*, const uint8_t*, int, int, int, int, int, int, int, bool);
    void nxImageCopy(NxImage*, const NxImage*, int, int, int, int, int, int, bool);
    void nxImageSetPixel(NxImage*, uint32_t, uint32_t, uint8_t, uint8_t, uint8_t, uint8_t);
    void nxImageGetPixel(const NxImage*, uint32_t, uint32_t, uint8_t*);
    const uint8_t* nxImageGetPixelsPtr(const NxImage*);
    void nxImageFlipHorizontally(NxImage*);
    void nxImageFlipVertically(NxImage*);
]]

------------------------------------------------------------
-- A class to handle Image creation and management
------------------------------------------------------------
local class = require 'nx.class'
local Image = class 'nx.image'

------------------------------------------------------------
local function isCArray(a)
    return type(a) == 'cdata' or type(a) == 'userdata'
end

------------------------------------------------------------
function Image:initialize(a, b, c, d, e, f)
    local handle = C.nxImageNew()
    self._cdata  = ffi.gc(handle, C.nxImageRelease)

    if type(a) == 'number' then
        self:create(a, b, c, d, e, f)
    elseif a then
        self:load(a, b)
    end
end

------------------------------------------------------------
function Image:create(width, height, r, g, b, a)
    if type(r) == 'table' then
        C.nxImageCreateFromData(self._cdata, width, height, ffi.new('const uint8_t[?]', #r, r))
    elseif isCArray(r) then
        C.nxImageCreateFromData(self._cdata, width, height, ffi.cast('const uint8_t*', r))
    else
        C.nxImageCreateFill(self._cdata, width, height, r or 255, g or 255, b or 255, a or 255)
    end

    return self
end

------------------------------------------------------------
function Image:load(a, b)
    local ok = false

    if isCArray(a) and type(b) == 'number' then -- Load from memory
        ok = C.nxImageOpenFromMemory(self._cdata, a, b)
    elseif type(a) == 'string' then -- Load from file
        ok = C.nxImageOpenFromFile(self._cdata, a)
    elseif a and a.class and a.class.name == 'nx.inputfile' then -- Load from handle
        ok = C.nxImageOpenFromHandle(self._data, a._cdata, false)
    end

    -- Appropriate error message for each case
    if not ok then
        print(self, self.create, 'um')
        self:create(2, 2, 255, 0, 255)
    end

    return self
end

------------------------------------------------------------
function Image:save(filename)
    if self._cdata == nil or not filename then return false end
    return C.nxImageSave(self._cdata, filename)
end

------------------------------------------------------------
function Image:release()
    if self._cdata == nil then return end
    C.nxImageRelease(ffi.gc(self._cdata, nil))
end

------------------------------------------------------------
function Image:size()
    local sizePtr = ffi.new('unsigned int[2]')
    C.nxImageGetSize(self._cdata, sizePtr)
    return tonumber(sizePtr[0]), tonumber(sizePtr[1])
end

------------------------------------------------------------
function Image:setColorMask(r, g, b, a, alpha)
    C.nxImageColorMask(self._cdata, r or 0, g or 0, b or 0, a or 255, alpha or 0)

    return self
end

------------------------------------------------------------
function Image:copy(source, a, b, c, d, e, f, g, h)
    if class.Object.isInstanceOf(source, Image) then
        C.nxImageCopy(self._cdata, source._cdata.img, a or 0, b or 0, c or 0, d or 0,
            e or 0, f or 0, not not g)
    else
        C.nxImageCopyPixels(self._cdata, source, a or 0, b or 0, c or 0, d or 0, e or 0, f or 0,
            g or 0, not not h)
    end

    return self
end

------------------------------------------------------------
function Image:setPixel(x, y, r, g, b, a)
    C.nxImageSetPixel(self._cdata, x, y, r, g, b, a)

    return self
end

------------------------------------------------------------
function Image:pixel(x, y)
    local colorPtr = ffi.new('uint8_t[4]')
    C.nxImageGetPixel(self._cdata, x, y, colorPtr)

    return tonumber(colorPtr[0]),
        tonumber(colorPtr[1]),
        tonumber(colorPtr[2]),
        tonumber(colorPtr[3])
end

------------------------------------------------------------
function Image:data()
    return C.nxImageGetPixelsPtr(self._cdata)
end

------------------------------------------------------------
function Image:flipHorizontally()
    C.nxImageFlipHorizontally(self._cdata)
end

------------------------------------------------------------
function Image:flipVertically()
    C.nxImageFlipVertically(self._cdata)
end

------------------------------------------------------------
return Image