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

local Matrix = require 'nx.matrix'

local Entity3D = {}

------------------------------------------------------------
function Entity3D:initialize()
    self._posX, self._posY, self._posZ = 0, 0, 0
    self._rotX, self._rotY, self._rotZ = 0, 0, 0
    self._scaleX, self._scaleY, self._scaleZ = 1, 1, 1
    self._originX, self._originY, self._originZ = 0, 0, 0

    self._colR, self._colG, self._colB, self._colA = 255, 255, 255, 255
end

------------------------------------------------------------
function Entity3D:setPosition(x, y, z)
    self._posX, self._posY, self._posZ = x, y, z

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Entity3D:setRotation(x, y, z)
    self._rotX, self._rotY, self._rotZ = x, y, z

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Entity3D:setScaling(x, y, z)
    self._scaleX, self._scaleY, self._scaleZ = x, y, z

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Entity3D:setOrigin(x, y, z)
    self._originX, self._originY, self._originZ = x, y, z

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Entity3D:move(x, y, z)
    return self:setPosition(self._posX + x, self._posY + y, self._posZ + z)
end

------------------------------------------------------------
function Entity3D:rotate(x, y, z)
    return self:setRotation(self._rotX + x, self._rotY + y, self._rotZ + z)
end

------------------------------------------------------------
function Entity3D:scale(x, y, z)
    return self:setScale(self._scaleX * x, self._scaleY * y, self._scaleZ * z)
end

------------------------------------------------------------
function Entity3D:offset(x, y, z)
    return self:setORigin(self._originX + x, self._originY + y, self._originZ + z)
end

------------------------------------------------------------
function Entity3D:lookAt(targetX, targetY, targetZ, upX, upY, upZ)
    self._targetX, self._targetY, self._targetZ = targetX, targetY, targetZ
    self._upX, self._upY, self._upZ = upX or 0, upY or 1, upZ or 0

    self._matrix = nil
    self._invMatrix = nil

    return self
end

------------------------------------------------------------
function Entity3D:position()
    return self._posX, self._posY, self._posZ
end

------------------------------------------------------------
function Entity3D:rotation()
    return self._rotX, self._rotY, self._rotZ
end

------------------------------------------------------------
function Entity3D:scaling()
    return self._scaleX, self._scaleY, self._scaleZ
end

------------------------------------------------------------
function Entity3D:origin()
    return self._originX, self._originY, self._originZ
end

------------------------------------------------------------
function Entity3D:target()
    if not self._tragetX then return nil end

    return self._targetX, self._targetY, self._tragetZ, self._upX, self._upY, self._upZ
end

------------------------------------------------------------
function Entity3D:matrix()

end

------------------------------------------------------------
function Entity3D:invMatrix()
    if not self._invMatrix then
        self._invMatrix = self._matrix:inverse()
    end

    return self._invMatrix
end

------------------------------------------------------------
function Entity3D:_render(camera, state)
    -- Nothing to do
end

------------------------------------------------------------
function Entity3D:_draw(camera, state)
    state:combineMatrix(self:matrix())
    -- state:combineColor(self:color())

    self:_render(camera, state)
end

------------------------------------------------------------
return Entity3D