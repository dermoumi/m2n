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

local class    = require 'class'
local Graphics = require 'graphics'

local Material = class 'graphics.material'

function Material.static.factory(task)
    task:addTask(true, function(mat, filename)
            local matData = loadfile(filename)
            if not matData then return false end

            matData = matData()
            if type(matData) ~= 'table' then return false end

            local retVals = {
                matData.shader
            }

            if matData.textures then
                retVals[#retVals+1] = '=#textures'
                for slot, id in pairs(matData.textures) do
                    retVals[#retVals+1] = slot
                    retVals[#retVals+1] = id
                end
            end

            if matData.uniforms then
                retVals[#retVals+1] = '=#uniforms'
                for uniform, data in pairs(matData.uniforms) do
                    retVals[#retVals+1] = uniform
                    retVals[#retVals+1] = data
                end
            end

            return unpack(retVals)
        end)
        :addTask(function(mat, filename, ...)
            local params, stage, key = {...}

            for i, param in ipairs(params) do
                if param == '=#textures' then
                    stage = '=#textures'
                elseif param == '=#uniforms' then
                    stage = '=#uniforms'
                elseif stage == '=#textures' then
                    if key then
                        mat:setTexture(param, key)
                        key = nil
                    else
                        key = param
                    end
                elseif stage == '=#uniforms' then
                    if key then
                        if type(param) == 'table' then
                            mat:setUniform(key, unpack(param))
                        else
                            mat:setUniform(key, param)
                        end
                        key = nil
                    else
                        key = param
                    end
                else
                    mat:setShader(param)
                end
            end
        end)
end

function Material:initialize(context)
    self._context = context or 'ambient'
    self._textures = {}
    self._uniforms = {}
    self._shader = Graphics.defaultShader(3)
end

function Material:setShader(shader)
    self._shader = shader or Graphics.defaultShader(3)

    return self
end

function Material:setTexture(texture, slot)
    self._textures[slot or 'uTexture0'] = texture

    return self
end

function Material:setContext(context)
    self._context = context

    return self
end

function Material:setUniform(name, a, b, c, d)
    self._uniforms[name] = {a, b, c, d}

    return self
end

function Material:shader()
    return self._shader
end

function Material:texture(slot)
    return self._textures[slot or 'uTexture0']
end

function Material:context()
    return self._context
end

function Material:_apply(projMat, transMat, context)
    if context and context ~= self._context then return false end

    self._shader:bind()
        :setUniform('uProjMat', projMat)
        :setUniform('uTransMat', transMat)

    for uniform, values in pairs(self._uniforms) do
        self._shader:setUniform(uniform, unpack(values))
    end

    local i = 0
    for slot, texture in pairs(self._textures) do
        texture:bind(i)
        self._shader:setSampler(slot, i)
        i = i + 1
    end

    if i == 0 then
        Graphics.defaultTexture():bind(0)
        self._shader:setSampler('uTexture0', 0)
    end

    return true
end

return Material
