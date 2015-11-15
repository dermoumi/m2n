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

------------------------------------------------------------
-- ffi C declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        uint32_t id;
    } NxShader;
]]

------------------------------------------------------------
-- A set of vertex and fragment shaders
------------------------------------------------------------
local class  = require 'nx.class'
local Shader = class 'nx.shader'

local Renderer  = require 'nx.renderer'
local InputFile = require 'nx.inputfile'
local Matrix4   = require 'nx.matrix4'

------------------------------------------------------------
local function destroy(cdata)
    if cdata.id ~= 0 then
        C.nxRendererDestroyShader(cdata.id)
    end

    C.free(cdata)
end

------------------------------------------------------------
function Shader.static._fromCData(cdata)
    local shader = Shader:allocate()
    shader._uniforms = {}
    shader._samplers = {}
    shader._cdata = ffi.cast('NxShader*', cdata)
    return shader
end

------------------------------------------------------------
function Shader:initialize()
    local handle = ffi.cast('NxShader*', C.malloc(ffi.sizeof('NxShader')))
    handle.id = 0
    self._cdata = ffi.gc(handle, destroy)

    self._uniforms = {}
    self._samplers = {}
end

------------------------------------------------------------
function Shader:release()
    destroy(ffi.gc(self._cdata, nil))
end

------------------------------------------------------------
function Shader:load(vertexShader, fragmentShader)
    C.nxRendererDestroyShader(self._cdata.id)

    local file, dontClose
    if class.Object.isInstanceOf(vertexShader, InputFile) then
        file = vertexShader
        file:seek(0)
        dontClose = true
    else
        file = InputFile:new(vertexShader)
    end

    if file then
        vertexShader = file:read()
        if not dontClose then file:close() end
        file = nil
    elseif type(vertexShader) ~= 'string' then
        -- Fall back to default vertex shader
        vertexShader = C.nxRendererDefaultVSShader()
    end

    if class.Object.isInstanceOf(fragmentShader, InputFile) then
        file = fragmentShader
        file:seek(0)
        dontClose = true
    else
        file = InputFile:new(fragmentShader)
    end

    if file then
        fragmentShader = file:read()
        if not dontClose then file:close() end
        file = nil
    elseif type(fragmentShader) ~= 'string' then
        -- Fall back to default fragment shader
        fragmentShader = C.nxRendererDefaultFSShader()
    end

    self._cdata.id = C.nxRendererCreateShader(vertexShader, fragmentShader)
    if self._cdata.id == 0 then
        return false, 'Cannot load shader: ' .. ffi.string(C.nxRendererGetShaderLog())
    end

    return true
end

------------------------------------------------------------
function Shader:bind()
    C.nxRendererBindShader(self._cdata.id)
end

------------------------------------------------------------
function Shader:setUniform(name, a, b, c, d)
    if not a then
        return false, 'Invalid parameters'
    end

    local prevShader = C.nxRendererGetCurrentShader()
    self:bind()

    local uniform = self._uniforms[name]

    if not uniform then
        uniform = C.nxRendererGetShaderConstLoc(self._cdata.id, name)
        self._uniforms[name] = uniform
    end

    if uniform == -1 then
        C.nxRendererBindShader(prevShader)
        return false, 'Uniform "' .. name .. '" does not exist'
    end

    local uniformType, uniformData
    if class.Object.isInstanceOf(a, Matrix4) then
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

    C.nxRendererSetShaderConst(uniform, uniformType, uniformData, 1)

    C.nxRendererBindShader(prevShader)
    return true
end

------------------------------------------------------------
function Shader:setSampler(name, sampler)
    if not sampler then
        return false, 'Invalid parameters'
    end

    local prevShader = C.nxRendererGetCurrentShader()
    self:bind()

    local uniform = self._samplers[name]

    if not uniform then
        uniform = C.nxRendererGetShaderSamplerLoc(self._cdata.id, name)
        self._samplers[name] = uniform
    end

    if uniform == -1 then
        C.nxRendererBindShader(prevShader)
        return false, 'Sampler "' .. name .. '"does not exist'
    end

    C.nxRendererSetShaderSampler(uniform, sampler)

    C.nxRendererBindShader(prevShader)
    return true
end

------------------------------------------------------------
return Shader