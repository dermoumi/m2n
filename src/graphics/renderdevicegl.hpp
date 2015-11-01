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

//==========================================================
// OpenGL implementation of RenderDevice
//==========================================================
class RenderDeviceGL : public RenderDevice
{
public:
    bool initialize();
    void clear(const float* color);

    void initStates();
    void resetStates();

    // Buffers
    void beginRendering();
    void finishRendering();
    uint32_t createVertexBuffer(uint32_t size, const void* data);
    uint32_t createIndexBuffer(uint32_t size, const void* data);

private:
    struct RDIBuffer
    {
        uint32_t type;
        uint32_t glObj;
        uint32_t size;
    };

private:
    RDIBuffer& getBuffer(uint32_t handle);

private:
    uint32_t mBufferMemory  {0u};

    int mDefaultFBO {0};
    RDIObjects<RDIBuffer> mBuffers;
};

//------------------------------------------------------------------------------
#endif
//==============================================================================
