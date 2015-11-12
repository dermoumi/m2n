/*//============================================================================
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
*///============================================================================
#include "../config.hpp"
#include "../system/log.hpp"

#if !defined(NX_OPENGL_ES)
    #include "../graphics/renderdevicegl.hpp"
#else
    #include "../graphics/renderdevicegles2.hpp"
#endif

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
struct NxVertexLayoutAttrib
{
    char     semanticName[32];
    uint32_t vbSlot;
    uint32_t size;
    uint32_t offset;
};

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
static RenderDevice* rdi {nullptr};
    
//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT bool nxRendererInit()
{
    // Delete the render device if reinitiazing
    if (rdi) delete rdi;

    // Instanciate the proper render device
    #if !defined(NX_OPENGL_ES)
        rdi = new RenderDeviceGL();
    #else
        rdi = new RenderDeviceGLES2();
    #endif

    return rdi->initialize();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererBegin()
{
    rdi->beginRendering();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererFinish()
{
    rdi->finishRendering();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererResetStates()
{
    rdi->resetStates();
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererCommitStates(uint32_t filter)
{
    return rdi->commitStates(filter);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth,
    bool col0, bool col1, bool col2, bool col3, bool dpth)
{
    uint32_t flags = 0;
    if (col0) flags |= RenderDevice::ClrColorRT0;
    if (col1) flags |= RenderDevice::ClrColorRT1;
    if (col2) flags |= RenderDevice::ClrColorRT2;
    if (col3) flags |= RenderDevice::ClrColorRT3;
    if (dpth) flags |= RenderDevice::ClrDepth;

    float clearColor[] = {r/255.f, g/255.f, b/255.f, a/255.f};

    rdi->clear(flags, clearColor, depth);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererDraw(uint32_t primType, uint32_t firstVert, uint32_t vertCount)
{
    rdi->draw(static_cast<RenderDevice::PrimType>(primType), firstVert, vertCount);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererDrawIndexed(uint32_t primType, uint32_t firstIndex, uint32_t indexCount)
{
    rdi->drawIndexed(static_cast<RenderDevice::PrimType>(primType), firstIndex, indexCount);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererRegisterVertexLayout(uint32_t numAttribs,
    const NxVertexLayoutAttrib* attribs)
{
    std::vector<RenderDevice::VertexLayoutAttrib> attributes(numAttribs);
    for (uint32_t i = 0; i < numAttribs; ++i) {
        attributes[i].semanticName = attribs[i].semanticName;
        attributes[i].vbSlot       = attribs[i].vbSlot;
        attributes[i].size         = attribs[i].size;
        attributes[i].offset       = attribs[i].offset;
    } 

    return rdi->registerVertexLayout(numAttribs, attributes.data());
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererCreateVertexBuffer(uint32_t size, const void* data)
{
    return rdi->createVertexBuffer(size, data);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererCreateIndexBuffer(uint32_t size, const void* data)
{
    return rdi->createIndexBuffer(size, data);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererDestroyBuffer(uint32_t buffer)
{
    rdi->destroyBuffer(buffer);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererUpdateBufferData(uint32_t buffer, uint32_t offset, uint32_t size,
    const void* data)
{
    return rdi->updateBufferData(buffer, offset, size, data);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererGetBufferMemory()
{
    return rdi->getBufferMemory();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendrerCalcTextureSize(uint32_t format, int width, int height, int depth)
{
    return rdi->calcTextureSize(
        static_cast<RenderDevice::TextureFormat>(format), width, height, depth
    );
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererCreateTexture(uint32_t type, int width, int height, unsigned int depth,
    uint32_t format, bool hasMips, bool genMips, bool sRGB)
{
    return rdi->createTexture(
        static_cast<RenderDevice::TextureType>(type), width, height, depth,
        static_cast<RenderDevice::TextureFormat>(format), hasMips, genMips, sRGB
    );
}

//----------------------------------------------------------
NX_EXPORT void nxRendererUploadTextureData(uint32_t texObj, int slice, int mipLevel,
    const void* pixels)
{
    rdi->uploadTextureData(texObj, slice, mipLevel, pixels);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererUploadTextureSubData(uint32_t texObj, int slice, int mipLevel,
    unsigned int x, unsigned int y, unsigned int z, unsigned int width, unsigned int height,
    unsigned int depth, const void* pixels)
{
    rdi->uploadTextureSubData(texObj, slice, mipLevel, x, y, z, width, height, depth, pixels);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererDestroyTexture(uint32_t texObj)
{
    rdi->destroyTexture(texObj);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetTextureData(uint32_t texObj, int slice, int mipLevel, void* buffer)
{
    return rdi->getTextureData(texObj, slice, mipLevel, buffer);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererGetTextureMemory()
{
    return rdi->getTextureMemory();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererCreateShader(const char* vertexShader, const char* fragmentShader)
{
    return rdi->createShader(vertexShader, fragmentShader);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererDestroyShader(uint32_t shader)
{
    rdi->destroyShader(shader);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererBindShader(uint32_t shader)
{
    rdi->bindShader(shader);
}

//----------------------------------------------------------
NX_EXPORT const char* nxRendererGetShaderLog()
{
    return rdi->getShaderLog().data();
}

//----------------------------------------------------------
NX_EXPORT int nxRendererGetShaderConstLoc(uint32_t shader, const char* name)
{
    return rdi->getShaderConstLoc(shader, name);
}

//----------------------------------------------------------
NX_EXPORT int nxRendererGetShaderSamplerLoc(uint32_t shader, const char* name)
{
    return rdi->getShaderSamplerLoc(shader, name);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetShaderConst(int loc, uint32_t type, float* values, uint32_t count)
{
    rdi->setShaderConst(loc, static_cast<RenderDevice::ShaderConstType>(type), values, count);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetShaderSampler(int loc, uint32_t texUnit)
{
    rdi->setShaderSampler(loc, texUnit);
}

//----------------------------------------------------------
NX_EXPORT const char* nxRendererGetDefaultVSCode()
{
    return rdi->getDefaultVSCode();
}

//----------------------------------------------------------
NX_EXPORT const char* nxRendererGetDefaultFSCode()
{
    return rdi->getDefaultFSCode();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererCreateRenderbuffer(uint32_t width, uint32_t height, uint32_t format,
    bool depth, uint32_t colBufs, uint32_t samples)
{
    return rdi->createRenderBuffer(
        width, height, static_cast<RenderDevice::TextureFormat>(format), depth, colBufs, samples
    );
}

//----------------------------------------------------------
NX_EXPORT void nxRendererDestroyRenderbuffer(uint32_t rb)
{
    rdi->destroyRenderBuffer(rb);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererGetRenderbufferTexture(uint32_t rb, uint32_t bufIndex)
{
    return rdi->getRenderBufferTexture(rb, bufIndex);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetRenderbuffer(uint32_t rb)
{
    rdi->setRenderBuffer(rb);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererGetRenderbufferSize(uint32_t rb, int* size)
{
    rdi->getRenderBufferSize(rb, &size[0], &size[1]);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetRenderbufferData(uint32_t rb, int bufIndex, int* size, int* compCount,
    void* buffer, int bufferSize)
{
    return rdi->getRenderBufferData(rb, bufIndex, &size[0], &size[1], compCount, buffer,
        bufferSize);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetViewport(int x, int y, int width, int height)
{
    rdi->setViewport(x, y, width, height);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetScissorRect(int x, int y, int width, int height)
{
    rdi->setScissorRect(x, y, width, height);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetIndexBuffer(uint32_t bufObj, int format)
{
    rdi->setIndexBuffer(bufObj, static_cast<RenderDevice::IndexFormat>(format));
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetVertexBuffer(uint32_t slot, uint32_t vbObj, uint32_t offset,
    uint32_t stride)
{
    rdi->setVertexBuffer(slot, vbObj, offset, stride);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetVertexLayout(uint32_t vlObj)
{
    rdi->setVertexLayout(vlObj);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetTexture(uint32_t slot, uint32_t texObj, uint16_t samplerState)
{
    rdi->setTexture(slot, texObj, samplerState);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetColorWriteMask(bool enabled)
{
    rdi->setColorWriteMask(enabled);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetColorWriteMask()
{
    return rdi->getColorWriteMask();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetFillMode(uint32_t fillMode)
{
    rdi->setFillMode(static_cast<RenderDevice::FillMode>(fillMode));
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererGetFillMode()
{
    return rdi->getFillMode();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetCullMode(uint32_t cullMode)
{
    rdi->setCullMode(static_cast<RenderDevice::CullMode>(cullMode));
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererGetCullMode()
{
    return rdi->getCullMode();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetScissorTest(bool enabled)
{
    rdi->setScissorTest(enabled);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetScissorTest()
{
    return rdi->getScissorTest();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetMultisampling(bool enabled)
{
    rdi->setMultisampling(enabled);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetMultisampling()
{
    return rdi->getMultisampling();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetAlphaToCoverage(bool enabled)
{
    rdi->setAlphaToCoverage(enabled);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetAlphaToCoverage()
{
    return rdi->getAlphaToCoverage();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetBlendMode(bool enabled, uint32_t src, uint32_t dst)
{
    rdi->setBlendMode(
        enabled, static_cast<RenderDevice::BlendFunc>(src),
        static_cast<RenderDevice::BlendFunc>(dst)
    );
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetBlendMode(uint32_t* blendFuncs)
{
    return rdi->getBlendMode(
        reinterpret_cast<RenderDevice::BlendFunc&>(blendFuncs[0]),
        reinterpret_cast<RenderDevice::BlendFunc&>(blendFuncs[1])
    );
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetDepthMask(bool enabled)
{
    rdi->setDepthMask(enabled);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetDepthMask()
{
    return rdi->getDepthMask();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetDepthTest(bool enabled)
{
    rdi->setDepthTest(enabled);
}

//----------------------------------------------------------
NX_EXPORT bool nxRendererGetDepthTest()
{
    return rdi->getDepthTest();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetDepthFunc(uint32_t func)
{
    rdi->setDepthFunc(static_cast<RenderDevice::DepthFunc>(func));
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxRendererGetDepthFunc()
{
    return rdi->getDepthFunc();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererGetCapabilities(unsigned int* maxTexUnits, unsigned int* maxTexSize,
    unsigned int* maxCubTexSize, unsigned int* maxColBufs, bool* dxt, bool* pvrtci, bool* etc1,
    bool* texFloat, bool* texDepth, bool* texSS, bool* tex3d, bool* texNPOT, bool* texSRGB,
    bool* rtms, bool* occQuery, bool* timerQuery)
{
    rdi->getCapabilities(
        maxTexUnits, maxTexSize, maxCubTexSize, maxColBufs, dxt, pvrtci, etc1,
        texFloat, texDepth, texSS, tex3d, texNPOT, texSRGB, rtms, occQuery, timerQuery
    );
}

//==============================================================================
