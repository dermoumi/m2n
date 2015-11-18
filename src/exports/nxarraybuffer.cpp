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
#include "../graphics/arraybuffer.hpp"

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxArraybuffer = Arraybuffer;

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT NxArraybuffer* nxArraybufferNew()
{
    return new NxArraybuffer();
}

//----------------------------------------------------------
NX_EXPORT void nxArraybufferRelease(NxArraybuffer* buffer)
{
    delete buffer;
}

//----------------------------------------------------------
NX_EXPORT bool nxArraybufferCreateVertex(NxArraybuffer* buffer, uint32_t size, const void* data)
{
    return buffer->createVertex(size, data);
}

//----------------------------------------------------------
NX_EXPORT bool nxArraybufferCreateIndex(NxArraybuffer* buffer, uint32_t size, const void* data)
{
    return buffer->createIndex(size, data);
}

//----------------------------------------------------------
NX_EXPORT bool nxArraybufferSetData(NxArraybuffer* buffer, uint32_t offset, uint32_t size,
    const void* data)
{
    return buffer->setData(offset, size, data);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxArraybufferUsedMemory()
{
    return NxArraybuffer::usedMemory();
}

//----------------------------------------------------------
NX_EXPORT void nxArraybufferSetVertexbuffer(const NxArraybuffer* buffer, uint8_t slot,
    uint32_t offset, uint32_t stride)
{
    NxArraybuffer::setVertexbuffer(buffer, slot, offset, stride);
}

//----------------------------------------------------------
NX_EXPORT void nxArraybufferSetIndexbuffer(const NxArraybuffer* buffer, uint8_t format)
{
    NxArraybuffer::setIndexbuffer(buffer, format);
}
