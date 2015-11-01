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
#pragma once
#include "../config.hpp"

#if !defined(NX_OPENGL_ES)
//------------------------------------------------------------------------------
#include "renderdevice.hpp"

#include <vector>

//==========================================================
// OpenGL implementation of RenderDevice
//==========================================================
class RenderDeviceGL : public RenderDevice
{
public:
    RenderDeviceGL();
    bool initialize();

    void initStates();
    void resetStates();
    bool commitStates(uint32_t filter = 0xFFFFFFFFu);

    // Drawcalls and clears
    void clear(const float* color);

    // Vertex layouts
    uint32_t registerVertexLayout(uint32_t numAttribs, const VertexLayoutAttrib* attribs);

    // Buffers
    void beginRendering();
    void finishRendering();
    uint32_t createVertexBuffer(uint32_t size, const void* data);
    uint32_t createIndexBuffer(uint32_t size, const void* data);
    void destroyBuffer(uint32_t buffer);
    bool updateBufferData(uint32_t buffer, uint32_t offset, uint32_t size, const void* data);
    uint32_t getBufferMemory() const;

    // GL States
    void setViewport(int x, int y, int width, int height);
    void setScissorRect(int x, int y, int width, int height);
    void setIndexBuffer(uint32_t bufObj, RDIIndexFormat idxFmt);
    void setVertexBuffer(uint32_t slot, uint32_t vbObj, uint32_t offset, uint32_t stride);
    void setVertexLayout(uint32_t vlObj);
    void setTexture(uint32_t slot, uint32_t texObj, uint16_t samplerState);

private:
    struct RDIBuffer
    {
        uint32_t type;
        uint32_t glObj;
        uint32_t size;
    };

    enum PendingMask : uint32_t
    {
        PMViewport     = 1 << 0,
        PMIndexBuffer  = 1 << 1,
        PMVertexLayout = 1 << 2,
        PMTextures     = 1 << 3,
        PMScissor      = 1 << 4,
        PMRenderStates = 1 << 5
    };

private:
    RDIBuffer& getBuffer(uint32_t handle);

private:
    int mVpX {0}, mVpY {0}, mVpWidth {1}, mVpHeight {1};
    int mScX {0}, mScY {0}, mScWidth {1}, mScHeight {1};
    uint32_t mBufferMemory  {0u};

    int mDefaultFBO {0};
    std::vector<RDIVertexLayout> mVertexLayouts;
    RDIObjects<RDIBuffer>        mBuffers;

    uint32_t mPendingMask {0xFFFFFFFFu};
};

//------------------------------------------------------------------------------
#endif
//==============================================================================
