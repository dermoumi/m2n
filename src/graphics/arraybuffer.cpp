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
#include "arraybuffer.hpp"
#include "renderdevice.hpp"

//----------------------------------------------------------
Arraybuffer::~Arraybuffer()
{
    if (mHandle) RenderDevice::instance().destroyBuffer(mHandle);
}

//----------------------------------------------------------
bool Arraybuffer::setData(uint32_t offset, uint32_t size, const void* data)
{
    return RenderDevice::instance().updateBufferData(mHandle, offset, size, data);
}

//----------------------------------------------------------
bool Arraybuffer::createVertex(uint32_t size, const void* data)
{
    if (mHandle) RenderDevice::instance().destroyBuffer(mHandle);
    mHandle = RenderDevice::instance().createVertexBuffer(size, data);
    return mHandle != 0;
}

//----------------------------------------------------------
bool Arraybuffer::createIndex(uint32_t size, const void* data)
{
    if (mHandle) RenderDevice::instance().destroyBuffer(mHandle);
    mHandle = RenderDevice::instance().createIndexBuffer(size, data);
    return mHandle != 0;
}

//----------------------------------------------------------
uint32_t Arraybuffer::usedMemory()
{
    return RenderDevice::instance().getBufferMemory();
}

//----------------------------------------------------------
void Arraybuffer::setVertexbuffer(const Arraybuffer* buffer, uint8_t slot, uint32_t offset,
    uint32_t stride)
{
    RenderDevice::instance().setVertexBuffer(
        slot, buffer ? buffer->mHandle : 0u, offset, stride
    );
}

//----------------------------------------------------------
void Arraybuffer::setIndexbuffer(const Arraybuffer* buffer, uint8_t format)
{
    RenderDevice::instance().setIndexBuffer(
        buffer ? buffer->mHandle : 0u, static_cast<RenderDevice::IndexFormat>(format)
    );
}
