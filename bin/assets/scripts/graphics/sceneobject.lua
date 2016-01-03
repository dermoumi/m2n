--[[
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
--]]

local Node = require 'graphics.node'

local SceneObject = Node:subclass 'graphics.sceneobject'

function SceneObject:initialize(type)
    Node.initialize(self)

    self.tx, self.ty, self.tz = 0, 0, 0
    self.rx, self.ry, self.rz = 0, 0, 0
    self.sx, self.sy, self.sz = 1, 1, 1

    self.type = type
end

function SceneObject:setTransformation(tx, ty, tz, rx, ry, rz, sx, sy, sz)
    self.tx, self.ty, self.tz = tx, ty, tz
    self.rx, self.ry, self.rz = rx, ry, rz
    self.sx, self.sy, self.sz = sx, sy, sz

    return self
end

function SceneObject:makeEntity(entity)
    entity:setPosition(self.tx, self.ty, self.tz)
        :setRotation(self.rx, self.ry, self.rz)
        :setScaling(self.sx, self.sy, self.sy)

    for name, child in pairs(self.children) do
        entity:attach(name, child:makeEntity())
    end

    return entity
end

return SceneObject