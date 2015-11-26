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
local Texture = require 'nx.texture'
local Texture2D = Texture:subclass('nx.texture2d')

------------------------------------------------------------
function Texture2D.static._fromCData(cdata)
    local texture = Texture2D:allocate()
    texture._cdata = require('ffi').cast('NxTexture*', cdata)
    return texture
end

------------------------------------------------------------
function Texture2D:initialize(width, height, hasMips, mipMap)
    Texture.initialize(self)

    if width and height then
        ok, err = self:create(width, height, hasMips, mipMap)
        if not ok then return nil, err end
    end
end

------------------------------------------------------------
function Texture2D:create(width, height, hasMips, mipMap)
    return Texture.create(self, '2d', width, height, 1, hasMips, mipMap)
end

------------------------------------------------------------
function Texture2D:load(image, hasMips, mipMap)
    local localImage = false
    if type(image) == 'string' then
        image, err = require('nx.image').load(image)
        if not image then return false, err end
        localImage = true
    end

    local width, height = image:size()

    ok, err = self:create(width, height, hasMips, mipMap)
    if not ok then return false, err end

    self:setData(image:data())

    if localImage then image:release() end

    return true
end

------------------------------------------------------------
function Texture2D:setData(data, a, b, c, d, e)
    local x, y, width, height, mipLevel
    if not b then
        x, y, width, height, mipLevel = -1, -1, -1, -1, a
    else
        x, y, width, height, mipLevel = a, b, c, d, e
    end

    Texture.setData(self, data, x, y, width, height, 1, mipLevel)
end

------------------------------------------------------------
function Texture2D:data(mipLevel)
    return Texture.data(self, 1, mipLevel)
end

------------------------------------------------------------
return Texture2D
