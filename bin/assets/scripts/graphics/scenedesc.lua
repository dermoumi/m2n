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

local class = require 'class'

local SceneDesc = class 'graphics.scenedesc'

local function parseObject(ret, data)
    for i, v in pairs(data) do
        if type(v) == 'table' then
            ret[#ret+1] = i

            local source = v[1] or 'scene'
            v[1] = nil

            if source:match('.:.') then
                ret[#ret+1] = '=#'
                ret[#ret+1] = source
            else
                ret[#ret+1] = '=!' .. source
            end

            parseObject(ret, v)
            ret[#ret+1] = '=!'
        else
            ret[#ret+1] = i
            ret[#ret+1] = v
        end
    end
end
SceneDesc.static._parseObject = parseObject

local function cloneObject(obj)
    if type(obj.clone) == 'function' then
        return obj:clone()
    else
        local newObj = obj.class:new()
        for i, v in pairs(obj) do
            newObj[i] = v
        end
        return newObj
    end
end
SceneDesc.static._cloneObject = cloneObject

function SceneDesc.static.factory(task)
    task:addTask(true, function(obj, filename)
            local objData, err = loadfile(filename)
            if not objData then
                error('Unable to load file \'' .. filename .. '\' : ' .. err)
            end

            objData = objData()
            if type(objData) ~= 'table' then
                error('Invalid object data: ' .. filename)
            end

            local ret = {}
            require('graphics.scenedesc')._parseObject(ret, objData)
            return unpack(ret)
        end)
        :addTask(function(obj, filename, ...)
            -- local SceneDesc = require('graphics.scenedesc')

            local stack, key, nextIsSource = {obj}, nil, false
            for i, param in ipairs({...}) do
                if param == '=#' then
                    nextIsSource = true
                elseif nextIsSource then
                    param = SceneDesc._cloneObject(param)
                    stack[#stack]:attach(key, param)
                    stack[#stack+1] = param
                    key = nil
                    nextIsSource = false
                elseif param == '=!' then
                    if stack[#stack]._validate and stack[#stack]:_validate() == false then
                        error('Attempting to attach an invalid object')
                    end
                    
                    stack[#stack] = nil
                    key = nil
                elseif key then
                    if type(param) == 'string' and param:match('^=!.') then
                        if not key then error('Attempting to attach scene object with no name') end

                        local kind, newObj = param:sub(3), nil
                        if kind == 'model' then
                            newObj = require('graphics.modeldesc'):new()
                        elseif kind == 'mesh' then
                            newObj = require('graphics.meshdesc'):new()
                        elseif kind == 'camera' then
                            newObj = require('graphics.cameradesc'):new()
                        else
                            -- Anything else can be a scene node... really...
                            newObj = SceneDesc:new()
                        end

                        stack[#stack]:attach(key, newObj)
                        stack[#stack+1] = newObj
                    elseif type(param) == 'table'
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

function SceneDesc:initialize(type)
    self.children = {}

    self.tx, self.ty, self.tz = 0, 0, 0
    self.rx, self.ry, self.rz = 0, 0, 0
    self.sx, self.sy, self.sz = 1, 1, 1

    self.type = type or 'scene'
end

function SceneDesc:setTransformation(tx, ty, tz, rx, ry, rz, sx, sy, sz)
    self.tx, self.ty, self.tz = tx, ty, tz
    self.rx, self.ry, self.rz = rx, ry, rz
    self.sx, self.sy, self.sz = sx, sy, sz

    return self
end

function SceneDesc:makeEntity(entity)
    entity = entity or require('graphics.scene'):new()

    entity:setPosition(self.tx, self.ty, self.tz)
        :setRotation(self.rx, self.ry, self.rz)
        :setScaling(self.sx, self.sy, self.sz)

    for name, child in pairs(self.children) do
        entity:attach(name, child:makeEntity())
    end

    return entity
end

function SceneDesc:attach(name, obj)
    self.children[name] = obj

    obj:attachedTo(self)
    self:attached(obj)

    return self
end

function SceneDesc:attachedTo(obj)
    -- Nothing to do
end

function SceneDesc:attached(obj)

end

return SceneDesc