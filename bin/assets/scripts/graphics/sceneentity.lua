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

local Entity3D = require 'graphics.entity3d'

local SceneEntity = Entity3D:subclass 'graphics.sceneentity'

function SceneEntity:initialize(a)
    self.drawables, self.lights = {}, {}

    if type(a) == 'string' then
        Entity3D.initialize(self, a)
    else
        Entity3D.initialize(self, 'scene')
        if a then self:load(a) end
    end
end

function SceneEntity:load(graph)
    return graph:makeEntity(self)
end

function SceneEntity:attached(node)
    if node.type == 'scene' or node.type == 'model' then
        self.drawables[node] = true
    elseif node.type == 'light' then
        self.lights[node] = true
    end
end

function SceneEntity:detached(node)
    self.drawables[node] = nil
    self.lights[node] = nil
end

function SceneEntity:_draw(camera, context)
    self:_render(camera, context)

    for drawable in pairs(self.drawables) do
        camera:draw(drawable, context)
    end
end

return SceneEntity