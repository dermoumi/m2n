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

local class     = require 'class'
local Log       = require 'util.log'
local Matrix    = require 'util.matrix'
local InputFile = require 'filesystem.inputfile'

local Shader = class 'graphics.shader'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxShader NxShader;

    NxShader* nxShaderNew();
    void nxShaderRelease(NxShader*);
    bool nxShaderLoad(NxShader*, const char*, const char*);
    void nxShaderSetUniform(NxShader*, int, uint8_t, float*);
    void nxShaderSetSampler(NxShader*, int, int);
    int nxShaderUniformLocation(const NxShader*, const char*);
    int nxShaderSamplerLocation(const NxShader*, const char*);
    const char* nxShaderLog();
    void nxShaderBind(const NxShader*);
    const char* nxShaderDefaultVSCode();
    const char* nxShaderDefaultFSCode();
]]

function Shader.static.factory(task)
    task:addTask('gpu', function(shader, filename)
            local shaders = loadfile(filename)
            if not shaders then return false end

            local vs, fs = shaders()
            shader:load(vs, fs)
        end)
end

function Shader.static.bind(shader)
    if shader then shader = shader._cdata end
    C.nxShaderBind(shader)

    return Shader
end

function Shader:initialize(vertexShader, fragmentShader)
    local handle = C.nxShaderNew()
    self._cdata = ffi.gc(handle, C.nxShaderRelease)

    self._uniforms = {}
    self._samplers = {}

    if vertexShader or fragmentShader then
        self:load(vertexShader, fragmentShader)
    end
end

function Shader:release()
    if self._cdata == nil then return end
    C.nxShaderRelease(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

function Shader:load(vertexShader, fragmentShader)
    if type(vertexShader) == 'string' then
        -- Try to open vertex shader file
        local file = InputFile:new()
            :onError(function() end)
            :open(vertexShader)

        if file:isOpen() then
            vertexShader = file:read()
            file:release()
        end
    else
        -- Fall back to default vertex shader
        vertexShader = C.nxShaderDefaultVSCode()
    end

    if type(fragmentShader) == 'string' then
        -- Try to open fragment shader file
        file = InputFile:new()
            :onError(function() end)
            :open(fragmentShader)

        if file:isOpen() then
            fragmentShader = file:read()
            file = nil
        end
    else
        -- Fall back to default fragment shader
        fragmentShader = C.nxShaderDefaultFSCode()
    end

    if not C.nxShaderLoad(self._cdata, vertexShader, fragmentShader) then
        Log.warning('Cannot load shader: ' .. ffi.string(C.nxShaderLog()))
        self._cdata = nil
    end

    return self
end

function Shader:bind()
    C.nxShaderBind(self._cdata)

    return self
end

function Shader:setUniform(name, a, b, c, d)
    if self._cdata ~= nil then
        local uniform = self._uniforms[name]

        if not uniform then
            uniform = C.nxShaderUniformLocation(self._cdata, name)
            self._uniforms[name] = uniform

            if uniform < 0 then
                Log.warning('Uniform "' .. name .. '" does not exist')
            end
        end

        if uniform >= 0 then
            local uniformType, uniformData
            if class.Object.isInstanceOf(a, Matrix) then
                uniformType = 4
                uniformData = a:data()
            elseif not b then
                uniformType = 0
                uniformData = ffi.new('float[1]', {a})
            elseif not c then
                uniformType = 1
                uniformData = ffi.new('float[2]', {a, b})
            elseif not d then
                uniformType = 2
                uniformData = ffi.new('float[3]', {a, b, c})
            else
                uniformType = 3
                uniformData = ffi.new('float[4]', {a, b, c, d})
            end

            C.nxShaderSetUniform(self._cdata, uniform, uniformType, uniformData)
        end
    end

    return self
end

function Shader:setSampler(name, sampler)
    if self._cdata ~= nil then
        -- Check local uniforms
        local uniform = self._samplers[name]
        if not uniform then
            uniform = C.nxShaderSamplerLocation(self._cdata, name)
            self._samplers[name] = uniform

            if uniform < 0 then
                Log.warning('Sampler "' .. name .. '" does not exist')
            end
        end

        if uniform >= 0 then
            C.nxShaderSetSampler(self._cdata, uniform, sampler)
        end
    end

    return self
end

return Shader