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
#include "../graphics/renderbuffer.hpp"
#include "../graphics/renderdevice.hpp"

using NxRenderBuffer = RenderBuffer;
using NxTexture      = Texture;

NX_EXPORT NxRenderBuffer* nxRenderBufferNew()
{
    return RenderDevice::instance().newRenderBuffer();
}

NX_EXPORT void nxRenderBufferRelease(NxRenderBuffer* buffer)
{
    delete buffer;
}

NX_EXPORT bool nxRenderBufferCreate(NxRenderBuffer* buffer, uint8_t format, uint16_t width,
    uint16_t height, bool depth, uint8_t colBufCount, uint8_t samples)
{
    return buffer->create(format, width, height, depth, colBufCount, samples);
}

NX_EXPORT NxTexture* nxRenderBufferTexture(NxRenderBuffer* buffer, uint8_t bufIndex)
{
    return buffer->texture(bufIndex);
}

NX_EXPORT void nxRenderBufferSize(const NxRenderBuffer* buffer, uint16_t* sizePtr)
{
    buffer->size(sizePtr[0], sizePtr[1]);
}

NX_EXPORT uint8_t nxRenderBufferFormat(const NxRenderBuffer* buffer)
{
    return buffer->format();
}

NX_EXPORT void nxRenderBufferBind(NxRenderBuffer* buffer)
{
    NxRenderBuffer::bind(buffer);
}
