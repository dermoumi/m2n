/*
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
*/

#include "../config.hpp"
#include "../system/log.hpp"
#include "../graphics/renderdevice.hpp"

struct NxVertexLayoutAttrib
{
    char    semanticName[32];
    uint8_t vbSlot;
    uint8_t size;
    uint8_t offset;
    uint8_t format;
};

NX_EXPORT bool nxRendererInit()
{
    return RenderDevice::instance().initialize();
}

NX_EXPORT void nxRendererBegin()
{
    RenderDevice::instance().beginRendering();
}

NX_EXPORT void nxRendererFinish()
{
    RenderDevice::instance().finishRendering();
}

NX_EXPORT void nxRendererResetStates()
{
    RenderDevice::instance().resetStates();
}

NX_EXPORT bool nxRendererCommitStates(uint32_t filter)
{
    return RenderDevice::instance().commitStates(filter);
}

NX_EXPORT void nxRendererClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a, float depth,
    bool col0, bool col1, bool col2, bool col3, bool clrDepth)
{
    uint32_t flags = 0;
    if (col0)     flags |= RenderDevice::ClrColorRT0;
    if (col1)     flags |= RenderDevice::ClrColorRT1;
    if (col2)     flags |= RenderDevice::ClrColorRT2;
    if (col3)     flags |= RenderDevice::ClrColorRT3;
    if (clrDepth) flags |= RenderDevice::ClrDepth;

    float clearColor[] = {r/255.f, g/255.f, b/255.f, a/255.f};

    RenderDevice::instance().clear(flags, clearColor, depth);
}

NX_EXPORT void nxRendererDraw(uint8_t primType, uint32_t firstVert, uint32_t vertCount)
{
    RenderDevice::instance().draw(static_cast<RenderDevice::PrimType>(primType), firstVert, vertCount);
}

NX_EXPORT void nxRendererDrawIndexed(uint8_t primType, uint32_t firstIndex, uint32_t indexCount)
{
    RenderDevice::instance().drawIndexed(static_cast<RenderDevice::PrimType>(primType), firstIndex, indexCount);
}

NX_EXPORT uint32_t nxRendererRegisterVertexLayout(uint8_t numAttribs,
    const NxVertexLayoutAttrib* attribs)
{
    std::vector<RenderDevice::VertexLayoutAttrib> attributes(numAttribs);
    for (uint8_t i = 0; i < numAttribs; ++i) {
        attributes[i].semanticName = attribs[i].semanticName;
        memcpy(&attributes[i].vbSlot, &attribs[i].vbSlot, sizeof(NxVertexLayoutAttrib) - 32);
    }

    return RenderDevice::instance().registerVertexLayout(numAttribs, attributes.data());
}

NX_EXPORT void nxRendererSetViewport(int x, int y, int width, int height)
{
    RenderDevice::instance().setViewport(x, y, width, height);
}

NX_EXPORT void nxRendererSetScissorRect(int x, int y, int width, int height)
{
    RenderDevice::instance().setScissorRect(x, y, width, height);
}

NX_EXPORT void nxRendererSetVertexLayout(uint8_t vlObj)
{
    RenderDevice::instance().setVertexLayout(vlObj);
}

NX_EXPORT void nxRendererSetTexture(uint8_t slot, uint32_t texObj, uint16_t samplerState)
{
    RenderDevice::instance().setTexture(slot, texObj, samplerState);
}

NX_EXPORT void nxRendererSetColorWriteMask(bool enabled)
{
    RenderDevice::instance().setColorWriteMask(enabled);
}

NX_EXPORT bool nxRendererGetColorWriteMask()
{
    return RenderDevice::instance().getColorWriteMask();
}

NX_EXPORT void nxRendererSetFillMode(uint8_t fillMode)
{
    RenderDevice::instance().setFillMode(static_cast<RenderDevice::FillMode>(fillMode));
}

NX_EXPORT uint8_t nxRendererGetFillMode()
{
    return RenderDevice::instance().getFillMode();
}

NX_EXPORT void nxRendererSetCullMode(uint8_t cullMode)
{
    RenderDevice::instance().setCullMode(static_cast<RenderDevice::CullMode>(cullMode));
}

NX_EXPORT uint8_t nxRendererGetCullMode()
{
    return RenderDevice::instance().getCullMode();
}

NX_EXPORT void nxRendererSetScissorTest(bool enabled)
{
    RenderDevice::instance().setScissorTest(enabled);
}

NX_EXPORT bool nxRendererGetScissorTest()
{
    return RenderDevice::instance().getScissorTest();
}

NX_EXPORT void nxRendererSetMultisampling(bool enabled)
{
    RenderDevice::instance().setMultisampling(enabled);
}

NX_EXPORT bool nxRendererGetMultisampling()
{
    return RenderDevice::instance().getMultisampling();
}

NX_EXPORT void nxRendererSetAlphaToCoverage(bool enabled)
{
    RenderDevice::instance().setAlphaToCoverage(enabled);
}

NX_EXPORT bool nxRendererGetAlphaToCoverage()
{
    return RenderDevice::instance().getAlphaToCoverage();
}

NX_EXPORT void nxRendererSetBlendMode(bool enabled, uint8_t src, uint8_t dst)
{
    RenderDevice::instance().setBlendMode(
        enabled, static_cast<RenderDevice::BlendFunc>(src),
        static_cast<RenderDevice::BlendFunc>(dst)
    );
}

NX_EXPORT bool nxRendererGetBlendMode(uint8_t* blendFuncs)
{
    return RenderDevice::instance().getBlendMode(
        reinterpret_cast<RenderDevice::BlendFunc&>(blendFuncs[0]),
        reinterpret_cast<RenderDevice::BlendFunc&>(blendFuncs[1])
    );
}

NX_EXPORT void nxRendererSetDepthMask(bool enabled)
{
    RenderDevice::instance().setDepthMask(enabled);
}

NX_EXPORT bool nxRendererGetDepthMask()
{
    return RenderDevice::instance().getDepthMask();
}

NX_EXPORT void nxRendererSetDepthTest(bool enabled)
{
    RenderDevice::instance().setDepthTest(enabled);
}

NX_EXPORT bool nxRendererGetDepthTest()
{
    return RenderDevice::instance().getDepthTest();
}

NX_EXPORT void nxRendererSetDepthFunc(uint8_t func)
{
    RenderDevice::instance().setDepthFunc(static_cast<RenderDevice::DepthFunc>(func));
}

NX_EXPORT uint8_t nxRendererGetDepthFunc()
{
    return RenderDevice::instance().getDepthFunc();
}

NX_EXPORT void nxRendererSync()
{
    RenderDevice::instance().sync();
}

NX_EXPORT void nxRendererGetCapabilities(uint32_t* u, bool* b)
{
    RenderDevice::instance().getCapabilities(
        &u[0], &u[1], &u[2], &u[3], &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7], &b[8],
        &b[9], &b[10], &b[11], &b[12]
    );
}
