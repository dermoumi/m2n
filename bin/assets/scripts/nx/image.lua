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

    NxImage* nxImageCreateFill(unsigned int, unsigned int, uint8_t r, uint8_t g, uint8_t b,
        uint8_t a);
    NxImage* nxImageCreateFromData(unsigned int, unsigned int, const uint8_t*);
    NxImage* nxImageCreateFromFile(const char* filename);
    NxImage* nxImageCreateFromMemory(const void*, size_t);
    NxImage* nxImageCreateFromHandle(PHYSFS_File*, bool close);
    bool nxImageSave(const void*, const char*);
    void nxImageRelease(NxImage*);
]]

------------------------------------------------------------
local function isNumber(val)
    return type(val) == 'number'
end

local function isNilOrNumber(val)
    return (val == nil) or (type(val) == 'number')
end

local function isCArray(a)
    return type(a) == 'string' or type(a) == 'cdata' or type(a) == 'userdata'
end

------------------------------------------------------------
-- A class to handle Image creation and management
------------------------------------------------------------
local class = require 'nx.class'
local Image = class 'Image'

------------------------------------------------------------
function Image.static._fromCData(data)
    local image = Image:new()
    image._cdata = ffi.cast('NxImage*', data)
    return image
end

------------------------------------------------------------
function Image.static.create(width, height, r, g, b, a)
    local image = Image:new()

    local ok, err = image:create(width, height, r, g, b, a)
    if not ok then
        return nil, err
    end

    return image
end

------------------------------------------------------------
function Image.static.load(a, b)
    local image = Image:new()

    local ok, err = image:load(a, b)
    if not ok then
        return nil, err
    end

    return image
end

------------------------------------------------------------
function Image:create(width, height, r, g, b, a)
    -- TODO: Release previous image if exists

    if not isNumber(width) or not isNumber(height) then
        return false, 'Invalid dimensions'
    end

    local handle
    if isNilOrNumber(r) and isNilOrNumber(g) and isNilOrNumber(b) and isNilOrNumber(a) then
        handle = C.nxImageCreateFill(width, height, r or 0, g or 0, b or 0, a or 255);
    elseif type(r) == 'table' then
        handle = C.nxImageCreateFromData(width, height, ffi.new('const uint8_t[?]', #r, r))
    elseif isCArray(r) then
        handle = C.nxImageCreateFromData(width, height, r)
    else
        return false, 'Invalid parameters'
    end

    if handle == nil then
        return false, 'Cannot create image'
    end

    self._cdata = ffi.gc(handle, C.nxImageRelease)
    return true
end

------------------------------------------------------------
function Image:load(a, b)
    local handle

    if isCArray(a) and isNumber(b) then
        -- Load from memory
        handle = C.nxImageCreateFromMemory(a, b)
    elseif type(a) == 'string' then
        -- Load from file
        handle = C.nxImageCreateFromFile(a)
    elseif class.Object.isInstanceOf(a, require('nx._binaryfile')) then
        -- Load from handle
        handle = C.nxImageCreateFromHandle(a._cdata, false)
    else
        return false, 'Invalid parameters'
    end

    -- Appropriate error message for each case
    if handle == nil then
        return false, 'Cannot load image'
    end

    self._cdata = ffi.gc(handle, C.nxImageRelease)
    return true
end

------------------------------------------------------------
function Image:save(filename)
    if not self._cdata or not filename then return false end
    return C.nxImageSave(self._cdata, filename)
end

------------------------------------------------------------
return Image