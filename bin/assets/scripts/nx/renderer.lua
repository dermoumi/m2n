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

local Renderer = {}

------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct {
        char    semanticName[32];
        uint8_t vbSlot;
        uint8_t size;
        uint8_t offset;
        uint8_t format;
    } NxVertexLayoutAttrib;

    bool nxRendererInit();
    void nxRendererBegin();
    void nxRendererFinish();
    void nxRendererResetStates();
    bool nxRendererCommitStates(uint32_t);
    void nxRendererClear(uint8_t, uint8_t, uint8_t, uint8_t, float, bool, bool, bool, bool, bool);
    void nxRendererDraw(uint8_t, uint32_t, uint32_t);
    void nxRendererDrawIndexed(uint8_t, uint32_t, uint32_t);
    uint32_t nxRendererRegisterVertexLayout(uint8_t, const NxVertexLayoutAttrib*);
    uint32_t nxRendererCreateVertexBuffer(uint32_t, const void*);
    uint32_t nxRendererCreateIndexBuffer(uint32_t, const void*);
    void nxRendererDestroyBuffer(uint32_t);
    bool nxRendererUpdateBufferData(uint32_t, uint32_t, uint32_t, const void*);
    uint32_t nxRendererGetBufferMemory();
    void nxRendererSetViewport(int, int, int, int);
    void nxRendererSetScissorRect(int, int, int, int);
    void nxRendererSetIndexBuffer(uint32_t, uint8_t);
    void nxRendererSetVertexBuffer(uint8_t, uint32_t, uint32_t, uint32_t);
    void nxRendererSetVertexLayout(uint8_t);
    void nxRendererSetTexture(uint8_t, uint32_t, uint16_t);
    void nxRendererSetColorWriteMask(bool);
    bool nxRendererGetColorWriteMask();
    void nxRendererSetFillMode(uint8_t);
    uint8_t nxRendererGetFillMode();
    void nxRendererSetCullMode(uint8_t);
    uint8_t nxRendererGetCullMode();
    void nxRendererSetScissorTest(bool);
    bool nxRendererGetScissorTest();
    void nxRendererSetMultisampling(bool);
    bool nxRendererGetMultisampling();
    void nxRendererSetAlphaToCoverage(bool);
    bool nxRendererGetAlphaToCoverage();
    void nxRendererSetBlendMode(bool, uint8_t, uint8_t);
    bool nxRendererGetBlendMode(uint8_t*);
    void nxRendererSetDepthMask(bool);
    bool nxRendererGetDepthMask();
    void nxRendererSetDepthTest(bool);
    bool nxRendererGetDepthTest();
    void nxRendererSetDepthFunc(uint8_t);
    uint8_t nxRendererGetDepthFunc();
    void nxRendererSync();
    void nxRendererGetCapabilities(uint32_t*, bool*);
]]

------------------------------------------------------------
local vertexLayouts = {}
local defaultShaders = {}
local identityMatrix = require('nx.matrix'):new()
local defaultTexture, vbFsQuad, caps

local toFillMode = {
    solid = 0,
    wireframe = 1
}
local fromFillMode = {
    [0] = 'solid',
    [1] = 'wireframe'
}

local toCullMode = {
    back = 0,
    front = 1,
    none = 2
}
local fromCullMode = {
    [0] = 'back',
    [1] = 'front',
    [2] = 'none'
}

local toBlendFactor = {
    zero        = 0,
    one         = 1,
    srcalpha    = 2,
    invsrcalpha = 3,
    dstcolor    = 4
}
local fromBlendFactor = {
    [0] = 'zero',
    [1] = 'one',
    [2] = 'srcalpha',
    [3] = 'invsrcalpha',
    [4] = 'dstcolor'
}

local toDepthFunc = {
    lequal = 0,
    less = 1,
    equal = 2,
    greater = 3,
    gequal = 4,
    always = 5
}
local fromDepthFunc = {
    [0] = 'lequal',
    [1] = 'less',
    [2] = 'equal',
    [3] = 'greater',
    [4] = 'gequal',
    [5] = 'always'
}

------------------------------------------------------------
function Renderer.init()
    if not C.nxRendererInit() then return error('Unable to initialize renderer') end

    -- Initialize vertex layouts
    vertexLayouts[1] = C.nxRendererRegisterVertexLayout(2, ffi.new('NxVertexLayoutAttrib[2]', {
        {'aPosition',  0, 2, 0, 0},
        {'aTexCoords', 0, 2, 8, 0}
    }))
    --
    vertexLayouts[2] = C.nxRendererRegisterVertexLayout(3, ffi.new('NxVertexLayoutAttrib[3]', {
        {'aPosition',  0, 2, 0,  0},
        {'aColor',     0, 4, 8,  1},
        {'aTexCoords', 0, 2, 12, 0}
    }))

    -- Initialize default shaders
    local Shader = require('nx.shader')
    defaultShaders[1] = Shader:new([[
        attribute vec2 aPosition;
        attribute vec2 aTexCoords;
        uniform mat4 uTransMat;
        uniform mat4 uProjMat;
        varying vec2 vTexCoords;
        void main() {
            vTexCoords  = aTexCoords;
            gl_Position = uProjMat * uTransMat * vec4(aPosition, 0.0, 1.0);
        }
    ]], [[
        uniform sampler2D uTexture;
        uniform vec2 uTexSize;
        uniform vec4 uColor;
        varying vec2 vTexCoords;
        void main() {
            gl_FragColor = texture2D(uTexture, vTexCoords / uTexSize) * uColor;
        }
    ]])
    --
    defaultShaders[2] = Shader:new([[
        attribute vec2 aPosition;
        attribute vec4 aColor;
        attribute vec2 aTexCoords;
        uniform mat4 uTransMat;
        uniform mat4 uProjMat;
        varying vec2 vTexCoords;
        varying vec4 vColor;
        void main() {
            vTexCoords  = aTexCoords;
            vColor      = aColor;
            gl_Position = uProjMat * uTransMat * vec4(aPosition, 0.0, 1.0);
        }
    ]], [[
        uniform sampler2D uTexture;
        uniform vec2 uTexSize;
        uniform vec4 uColor;
        varying vec2 vTexCoords;
        varying vec4 vColor;
        void main() {
            gl_FragColor = texture2D(uTexture, vTexCoords / uTexSize) * uColor * vColor;
        }
    ]])

    -- Create the Fullscreen quad vertex buffer
    local buffer = ffi.new('float[12]', {
        -1,  1, 0, 0,
        -1, -3, 0, 2,
         3,  1, 2, 0
    })
    vbFsQuad = require('nx.arraybuffer').vertexbuffer(ffi.sizeof(buffer), buffer)

    -- Create the default, empty texture
    defaultTexture = require('nx.texture'):new()
        :create('2d', 1, 1, 1, 0, 0)
        :setData(ffi.new('uint8_t[4]', {255, 255, 255, 255}), 0, 0)
end

------------------------------------------------------------
function Renderer.begin()
    C.nxRendererBegin()

    return Renderer
end

------------------------------------------------------------
function Renderer.finish()
    C.nxRendererFinish()

    return Renderer
end

------------------------------------------------------------
function Renderer.getCapabilities(cap)
    if not caps then
        caps = {}

        local u, b = ffi.new('unsigned int[4]'), ffi.new('bool[12]')
        C.nxRendererGetCapabilities(u, b)

        caps.maxTexUnits     = tonumber(u[0])
        caps.maxTexSize      = tonumber(u[1])
        caps.maxCubeTexSize  = tonumber(u[2])
        caps.maxColorBuffers = tonumber(u[3])
        caps.dxtSupported             = b[0]
        caps.pvrtciSupported          = b[1]
        caps.etc1Supported            = b[2]
        caps.floatTexturesSupported   = b[3]
        caps.textureDepthSupported    = b[4]
        caps.textureShadowSampling    = b[5]
        caps.texture3DSupported       = b[6]
        caps.npotTexturesSupported    = b[7]
        caps.sRGBTexturesSupported    = b[8]
        caps.rtMultisamplingSupported = b[9]
        caps.occQueriesSupported      = b[10]
        caps.timerQueriesSupported    = b[11]
    end
    
    if not cap then
        local capsClone = {}
        for i, v in pairs(caps) do
            capsClone[i] = v
        end
        return capsClone
    end

    return caps[cap]
end

------------------------------------------------------------
function Renderer.vertexLayout(index)
    return vertexLayouts[index]
end

------------------------------------------------------------
function Renderer.defaultShader(index)
    return defaultShaders[index]
end

------------------------------------------------------------
function Renderer.defaultTexture()
    return defaultTexture
end

------------------------------------------------------------
function Renderer.drawFsQuad(texture, width, height)
    if width and height then
        local texW, texH = texture:size()
        width, height = texW / width, texH / height
    else
        width, height = 1, 1
    end

    texture:bind(0)

    local shader = defaultShaders[1]
    shader:bind()
    shader:setUniform('uProjMat', identityMatrix)
    shader:setUniform('uTransMat', identityMatrix)
    shader:setUniform('uColor', 1, 1, 1, 1)
    shader:setUniform('uTexSize', width, height)
    shader:setSampler('uTexture', 0)

    local Arraybuffer = require 'nx.arraybuffer'
    Arraybuffer.setVertexbuffer(vbFsQuad, 0, 0, 16)
    C.nxRendererSetVertexLayout(vertexLayouts[1])

    C.nxRendererDraw(4, 0, 3)

    return Renderer
end

------------------------------------------------------------
function Renderer.fillFsQuad(r, g, b, a, blendMode)
    defaultTexture:bind(0)

    Renderer.setBlendMode(blendMode or 'alpha')

    local shader = defaultShaders[1]
    shader:bind()
    shader:setUniform('uProjMat', identityMatrix)
    shader:setUniform('uTransMat', identityMatrix)
    shader:setUniform('uColor', (r or 0)/255, (g or 0)/255, (b or 0)/255, (a or 255)/255)
    shader:setUniform('uTexSize', 1, 1)
    shader:setSampler('uTexture', 0)

    local Arraybuffer = require 'nx.arraybuffer'
    Arraybuffer.setVertexbuffer(vbFsQuad, 0, 0, 16)
    C.nxRendererSetVertexLayout(vertexLayouts[1])

    C.nxRendererDraw(4, 0, 3)

    return Renderer
end

------------------------------------------------------------
function Renderer.enableColorWriteMask(enabled)
    C.nxRendererSetColorWriteMask(enabled)

    return Renderer
end

------------------------------------------------------------
function Renderer.colorWriteMaskEnabled()
    return C.nxRendererGetColorWriteMask()
end

------------------------------------------------------------
function Renderer.setFillMode(mode)
    C.nxRendererSetFillMode(toFillMode[mode])

    return Renderer
end

------------------------------------------------------------
function Renderer.fillMode()
    return fromFillMode[C.nxRendererGetFillMode()]
end

------------------------------------------------------------
function Renderer.setCullMode(mode)
    C.nxRendererSetCullMode(toCullMode[mode])

    return Renderer
end

------------------------------------------------------------
function Renderer.cullMode()
    return fromCullMode[C.nxRendererGetCullMode()]
end

------------------------------------------------------------
function Renderer.enableScissorTest(enabled)
    C.nxRendererSetScissorTest(enabled)

    return Renderer
end

------------------------------------------------------------
function Renderer.scissorTestEnabled()
    return C.nxRendererGetScissorTest()
end

------------------------------------------------------------
function Renderer.enableMultisampling(enabled)
    C.nxRendererSetMultisampling(enabled)

    return Renderer
end

------------------------------------------------------------
function Renderer.multisamplingEnabled()
    return C.nxRendererGetMultisampling()
end

------------------------------------------------------------
function Renderer.enableAlphaToCoverage(enabled)
    C.nxRendererSetAlphaToCoverage(enabled)

    return Renderer
end

------------------------------------------------------------
function Renderer.alphaToCoverageEnabled()
    return C.nxRendererGetAlphaToCoverage()
end

------------------------------------------------------------
function Renderer.setBlendMode(srcFactor, dstFactor)
    if srcFactor == 'none' or (srcFactor == 'one' and dstFactor == 'zero') then
        C.nxRendererSetBlendMode(false, 1, 0)
    elseif srcFactor == 'alpha' then
        C.nxRendererSetBlendMode(true, 2, 3)
    elseif srcFactor == 'add' then
        C.nxRendererSetBlendMode(true, 1, 1)
    elseif srcFactor == 'multiply' then
        C.nxRendererSetBlendMode(true, 4, 0)
    else
        C.nxRendererSetBlendMode(true, toBlendFactor[srcFactor], toBlendFactor[dstFactor])
    end

    return Renderer
end

------------------------------------------------------------
function Renderer.blendMode()
    local factors = ffi.new('uint32_t[2]')
    if C.nxRendererGetBlendMode(factors) then
        return true, fromBlendFactor[factors[0]], fromBlendFactor[factors[1]]
    end
    
    return false
end

------------------------------------------------------------
function Renderer.enabledDepthMask(enabled)
    C.nxRendererSetDepthMask(enabled)

    return Renderer
end

------------------------------------------------------------
function Renderer.depthMaskEnabled()
    return C.nxRendererGetDepthMask()
end

------------------------------------------------------------
function Renderer.enabledDepthTest(enabled)
    C.nxRendererSetDepthTest(enabled)

    return Renderer
end

------------------------------------------------------------
function Renderer.depthTestEnabled()
    return C.nxRendererGetDepthTest()
end

------------------------------------------------------------
function Renderer.setDepthFunc(func)
    C.nxRendererSetDepthFunc(toDepthFunc[func])

    return Renderer
end

------------------------------------------------------------
function Renderer.depthFunc()
    return fromDepthFunc[C.nxRendererGetDepthFunc()]
end

------------------------------------------------------------
function Renderer.sync()
    C.nxRendererSync()

    return Renderer
end

------------------------------------------------------------
return Renderer