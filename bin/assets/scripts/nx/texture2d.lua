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

local Image   = require 'nx.image'
local Texture = require 'nx.texture'

local Texture2D = Texture:subclass('nx.texture2d')

------------------------------------------------------------
function Texture2D:initialize(a, b, c, d)
    Texture.initialize(self)

    if type(a) == 'number' then
        self:create(a, b, c, d)
    elseif a then
        self:load(a, b, c)
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
        image = Image:new(image)
        if not image then return false, err end
        localImage = true
    end

    local width, height = image:size()

    self:create(width, height, hasMips, mipMap)
        :setData(image:data())

    if localImage then image:release() end

    return self
end

------------------------------------------------------------
function Texture2D:setData(data, a, b, c, d, e)
    local x, y, width, height, mipLevel
    if not b then
        x, y, width, height, mipLevel = -1, -1, -1, -1, a
    else
        x, y, width, height, mipLevel = a, b, c, d, e
    end

    return Texture.setData(self, data, x, y, width, height, 1, mipLevel)
end

------------------------------------------------------------
function Texture2D:data(mipLevel)
    return Texture.data(self, 1, mipLevel)
end

------------------------------------------------------------
return Texture2D