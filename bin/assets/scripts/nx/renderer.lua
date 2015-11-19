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
        char     semanticName[32];
        uint8_t vbSlot;
        uint8_t size;
        uint8_t offset;
        uint8_t format;
    } NxVertexLayoutAttrib;

    typedef void* NxArrayBuffer;

    bool nxRendererInit();
    void nxRendererBegin();
    void nxRendererFinish();
    void nxRendererResetStates();
    bool nxRendererCommitStates(uint32_t);
    void nxRendererClear(uint8_t, uint8_t, uint8_t, uint8_t, float, bool, bool, bool, bool, bool);
    void nxRendererDraw(uint32_t, uint32_t, uint32_t);
    void nxRendererDrawIndexed(uint32_t, uint32_t, uint32_t);
    uint32_t nxRendererRegisterVertexLayout(uint8_t, const NxVertexLayoutAttrib*);
    NxArrayBuffer* nxRendererCreateVertexBuffer(uint32_t, const void*);
    NxArrayBuffer* nxRendererCreateIndexBuffer(uint32_t, const void*);
    void nxRendererDestroyBuffer(NxArrayBuffer*);
    bool nxRendererUpdateBufferData(NxArrayBuffer*, uint32_t, uint32_t, const void*);
    uint32_t nxRendererGetBufferMemory();
    void nxRendererSetViewport(int, int, int, int);
    void nxRendererSetScissorRect(int, int, int, int);
    void nxRendererSetIndexBuffer(NxArrayBuffer*, int);
    void nxRendererSetVertexBuffer(uint32_t, NxArrayBuffer*, uint32_t, uint32_t);
    void nxRendererSetVertexLayout(uint32_t);
    void nxRendererSetTexture(uint32_t, uint32_t, uint16_t);
    void nxRendererSetColorWriteMask(bool);
    bool nxRendererGetColorWriteMask();
    void nxRendererSetFillMode(uint32_t);
    uint32_t nxRendererGetFillMode();
    void nxRendererSetCullMode(uint32_t);
    uint32_t nxRendererGetCullMode();
    void nxRendererSetScissorTest(bool);
    bool nxRendererGetScissorTest();
    void nxRendererSetMultisampling(bool);
    bool nxRendererGetMultisampling();
    void nxRendererSetAlphaToCoverage(bool);
    bool nxRendererGetAlphaToCoverage();
    void nxRendererSetBlendMode(bool, uint32_t, uint32_t);
    bool nxRendererGetBlendMode(uint32_t*);
    void nxRendererSetDepthMask(bool);
    bool nxRendererGetDepthMask();
    void nxRendererSetDepthTest(bool);
    bool nxRendererGetDepthTest();
    void nxRendererSetDepthFunc(uint32_t);
    uint32_t nxRendererGetDepthFunc();
    void nxRendererGetCapabilities(unsigned int*, bool*);
]]

------------------------------------------------------------
-- A set of functions about on-screen rendering
------------------------------------------------------------
local Renderer = {}

------------------------------------------------------------
local caps = {}
local vertexLayouts = {}
local defaultShaders = {}

local vbFsQuad;

------------------------------------------------------------
function Renderer.init()
    if not C.nxRendererInit() then return false end

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

    -- Initialize vertex layouts
    vertexLayouts[1] = C.nxRendererRegisterVertexLayout(2, ffi.new('NxVertexLayoutAttrib[2]', {
        {'aPosition',  0, 2, 0, 0},
        {'aTexCoords', 0, 2, 8, 0}
    }))

    -- Initialize default shaders
    local Shader = require('nx.shader')
    defaultShaders[1] = Shader:new([[
        attribute vec2 aPosition;
        attribute vec2 aTexCoords;
        uniform mat4 uTransMat;
        varying vec2 vTexCoords;
        void main() {
            vTexCoords  = aTexCoords;
            gl_Position = uTransMat * vec4(aPosition, 0.0, 1.0);
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

    -- Create the Fullscreen quad vertex buffer
    local buffer = ffi.new('float[12]', {
        -1,  1, 0, 0,
         3,  1, 2, 0,
        -1, -3, 0, 2
    })
    vbFsQuad = require('nx.arraybuffer').vertexbuffer(ffi.sizeof(buffer), buffer)

    return true
end

------------------------------------------------------------
function Renderer.begin()
    C.nxRendererBegin()
end

------------------------------------------------------------
function Renderer.finish()
    C.nxRendererFinish()
end

------------------------------------------------------------
function Renderer.getCapabilities(cap)
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
    shader:setUniform('uTransMat', require('nx.matrix'):new())
    shader:setUniform('uColor', 1, 1, 1, 1)
    shader:setUniform('uTexSize', width, height)
    shader:setSampler('uTexture', 0)

    local Arraybuffer = require 'nx.arraybuffer'
    Arraybuffer.setVertexbuffer(vbFsQuad, 0, 0, 16)
    Arraybuffer.setIndexbuffer(nil)
    C.nxRendererSetVertexLayout(vertexLayouts[1])

    C.nxRendererDraw(0, 0, 3)
end

------------------------------------------------------------
return Renderer