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
        uint32_t vbSlot;
        uint32_t size;
        uint32_t offset;
    } NxVertexLayoutAttrib;

    bool nxRendererInit();
    void nxRendererBegin();
    void nxRendererFinish();
    void nxRendererResetStates();
    bool nxRendererCommitStates(uint32_t);
    void nxRendererClear(uint8_t, uint8_t, uint8_t, uint8_t, float, bool, bool, bool, bool, bool);
    void nxRendererDraw(uint32_t, uint32_t, uint32_t);
    void nxRendererDrawIndexed(uint32_t, uint32_t, uint32_t);
    uint32_t nxRendererRegisterVertexLayout(uint32_t, const NxVertexLayoutAttrib*);
    uint32_t nxRendererCreateVertexBuffer(uint32_t, const void*);
    uint32_t nxRendererCreateIndexBuffer(uint32_t, const void*);
    void nxRendererDestroyBuffer(uint32_t);
    bool nxRendererUpdateBufferData(uint32_t, uint32_t, uint32_t, const void*);
    uint32_t nxRendererGetBufferMemory();
    uint32_t nxRendererCalcTextureSize(uint32_t, int, int, int);
    uint32_t nxRendererCreateTexture(uint32_t, int, int, unsigned int, uint32_t, bool, bool, bool);
    void nxRendererUploadTextureData(uint32_t, int, int, const void*);
    void nxRendererUploadTextureSubData(uint32_t, int, int, unsigned int, unsigned int,
        unsigned int, unsigned int, unsigned int, unsigned int, const void*);
    void nxRendererDestroyTexture(uint32_t);
    bool nxRendererGetTextureData(uint32_t, int, int, void*);
    uint32_t nxRendererGetTextureMemory();
    uint32_t nxRendererCreateShader(const char*, const char*);
    void nxRendererDestroyShader(uint32_t);
    void nxRendererBindShader(uint32_t);
    const char* nxRendererGetShaderLog();
    int nxRendererGetShaderConstLoc(uint32_t, const char*);
    int nxRendererGetShaderSamplerLoc(uint32_t, const char*);
    void nxRendererSetShaderConst(int, uint32_t, float*, uint32_t);
    void nxRendererSetShaderSampler(int, uint32_t);
    const char* nxRendererGetDefaultVSCode();
    const char* nxRendererGetDefaultFSCode();
    uint32_t nxRendererCreateRenderbuffer(uint32_t, uint32_t, uint32_t, bool, uint32_t, uint32_t);
    void nxRendererDestroyRenderbuffer(uint32_t);
    uint32_t nxRendererGetRenderbufferTexture(uint32_t, uint32_t);
    void nxRendererSetRenderbuffer(uint32_t);
    void nxRendererGetRenderbufferSize(uint32_t, int*);
    bool nxRendererGetRenderbufferData(uint32_t, int, int*, int*, void*, int);
    void nxRendererSetViewport(int, int, int, int);
    void nxRendererSetScissorRect(int, int, int, int);
    void nxRendererSetIndexBuffer(uint32_t, int);
    void nxRendererSetVertexBuffer(uint32_t, uint32_t, uint32_t, uint32_t);
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

    return true
end

------------------------------------------------------------
function Renderer.clear(r, g, b, a, depth, col0, col1, col2, col3, clearDepth)
    if (col0 == nil) then col0 = true end
    if (col1 == nil) then col1 = true end
    if (col2 == nil) then col2 = true end
    if (col3 == nil) then col3 = true end
    if (clearDepth == nil) then clearDepth = true end

    C.nxRendererClear(
        r or 0, g or 0, b or 0, a or 255, dethp or 1.0, col0, col1, col2, col3, clearDepth
    )
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
function Renderer.setupViewport(x, y, w, h)
    C.nxRendererSetViewport(x, y, w, h)
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
return Renderer