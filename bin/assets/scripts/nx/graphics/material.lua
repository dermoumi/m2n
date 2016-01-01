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

local class    = require 'nx.class'
local Graphics = require 'nx.graphics'

local Material = class 'nx.graphics.material'

------------------------------------------------------------
function Material:initialize(context)
    self._context = context or 'ambient'
    self._textures = {}
    self._uniforms = {}
    self._shader = Graphics.defaultShader(3)
    self._colR, self._colG, self._colB, self._colA = 255, 255, 255, 255
end

------------------------------------------------------------
function Material:setShader(shader)
    self._shader = shader or Graphics.defaultShader(3)

    return self
end

------------------------------------------------------------
function Material:setTexture(texture, slot)
    self._textures[slot or 'uTexture'] = texture

    return self
end

------------------------------------------------------------
function Material:setColor(r, g, b, a)
    self._colR, self._colG, self._colB, self._colA = r, g, b, a or 255

    return self
end

------------------------------------------------------------
function Material:setContext(context)
    self._context = context

    return self
end

------------------------------------------------------------
function Material:setUniform(name, a, b, c, d)
    self._uniforms[name] = {a, b, c, d}

    return self
end

------------------------------------------------------------
function Material:shader()
    return self._shader
end

------------------------------------------------------------
function Material:texture(slot)
    return self._textures[slot or 'uTexture']
end

------------------------------------------------------------
function Material:color()
    return self._colR, self._colG, self._colB, self._colA
end

------------------------------------------------------------
function Material:context()
    return self._context
end

------------------------------------------------------------
function Material:_apply(projMat, transMat)
    self._shader:bind()
        :setUniform('uProjMat', projMat)
        :setUniform('uTransMat', transMat)
        :setUniform('uColor', self._colR/255, self._colG/255, self._colB/255, self._colA/255)

    for uniform, values in pairs(self._uniforms) do
        self._shader:setUniform(uniform, unpack(values))
    end

    local i = 0
    for slot, texture in pairs(self._textures) do
        texture:bind(i)
        shader:setSampler(slot, i)
        i = i + 1
    end

    if i == 0 then
        Graphics.defaultTexture():bind(0)
        self._shader:setSampler('uTexture', 0)
    end
end

------------------------------------------------------------
return Material