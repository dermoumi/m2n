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

local class       = require 'class'
local Graphics    = require 'graphics'

local Mesh = class 'graphics.mesh'

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

------------------------------------------------------------
function Mesh:initialize(material, start, count)
    self._material, self._start, self._count = material, start, count
end

------------------------------------------------------------
function Mesh:setMaterial(material)
    self._material = material

    return self
end

------------------------------------------------------------
function Mesh:material()
    return self._material
end

------------------------------------------------------------
function Mesh:_draw(projMat, transMat, context, indexed)
    if self._material and self._material._context == context then
        self._material:_apply(projMat, transMat)
        if indexed then
            C.nxRendererDrawIndexed(4, self._start, self._count)
        else
            C.nxRendererDraw(4, self._start, self._count)
        end
    end
end

------------------------------------------------------------
return Mesh