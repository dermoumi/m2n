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

local function parseObject(ret, data)
    for i, v in pairs(data) do
        if type(v) == 'table' then
            ret[#ret+1] = i
            ret[#ret+1] = '=!' .. v[1]
            v[1] = nil
            parseObject(ret, v)
            ret[#ret+1] = '=!'
        else
            ret[#ret+1] = i
            ret[#ret+1] = v
        end
    end
end
SceneObject.static._parseObject = parseObject

function SceneObject.static.factory(task)
    task:addTask(true, function(obj, filename)
            local objData = loadfile(filename)
            if not objData then return false end

            objData = objData()
            if type(objData) ~= 'table' then return false end

            local ret = {}
            require('graphics.sceneobject')._parseObject(ret, objData)
            return unpack(ret)
        end)
        :addTask(function(obj, filename, ...)
            local SceneObject = require('graphics.sceneobject')

            local stack = {obj}
            local key = nil
            for i, param in ipairs({...}) do
                if param == '=!' then
                    if stack[#stack]._validate then stack[#stack]:_validate() end
                    stack[#stack] = nil
                    key = nil
                elseif type(param) == 'string' and param:match('^=!.') then
                    if not key then return false end

                    local kind, newObj = param:sub(3), nil
                    if kind == 'scene' then
                        newObj = SceneObject:new()
                    elseif kind == 'model' then
                        newObj = require('graphics.model'):new()
                    elseif kind == 'mesh' then
                        newObj = require('graphics.mesh'):new()
                    else
                        return false
                    end

                    stack[#stack]:attach(key, newObj)
                    stack[#stack+1] = newObj
                    key = nil
                elseif key then
                    if type(param) == 'table'
                        and param.isInstanceOf
                        and param:isInstanceOf(SceneObject)
                    then
                        stack[#stack]:attach(key, param)
                    else
                        stack[#stack][key] = param
                    end
                    key = nil
                else
                    key = param
                end
            end
        end)
end

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