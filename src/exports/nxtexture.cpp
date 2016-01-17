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
#include "../graphics/texture.hpp"
#include "../graphics/renderdevice.hpp"

using NxTexture = Texture;

NX_EXPORT NxTexture* nxTextureNew()
{
    return RenderDevice::instance().newTexture();
}

NX_EXPORT void nxTextureRelease(NxTexture* texture)
{
    delete texture;
}

NX_EXPORT bool nxTextureCreate(NxTexture* texture, uint8_t type, uint8_t format, uint16_t width,
    uint16_t height, bool hasMips, bool mipMaps, bool srgb)
{
    return texture->create(
        static_cast<Texture::Type>(type), static_cast<Texture::Format>(format),
        width, height, hasMips, mipMaps, srgb
    );
}

NX_EXPORT void nxTextureSetData(NxTexture* texture, const void* buffer, uint8_t slice,
    uint8_t level)
{
    texture->setData(buffer, slice, level);
}

NX_EXPORT void nxTextureSetSubData(NxTexture* texture, const void* buffer, uint16_t x, uint16_t y,
    uint16_t width, uint16_t height, uint8_t slice, uint8_t level)
{
    texture->setSubData(buffer, x, y, width, height, slice, level);
}

NX_EXPORT bool nxTextureData(const NxTexture* texture, void* buffer, uint8_t slice, uint8_t level)
{
    return texture->data(buffer, slice, level);
}

NX_EXPORT void nxTextureSize(const NxTexture* texture, uint16_t* sizePtr)
{
    texture->size(sizePtr[0], sizePtr[1]);
}

NX_EXPORT uint32_t nxTextureBufferSize(const NxTexture* texture)
{
    return texture->bufferSize();
}

NX_EXPORT void nxTextureSetFilter(NxTexture* texture, uint32_t filter)
{
    texture->setFilter(static_cast<Texture::Filter>(filter));
}

NX_EXPORT void nxTextureSetAnisotropyLevel(NxTexture* texture, uint32_t aniso)
{
    texture->setAnisotropyLevel(static_cast<Texture::Anisotropy>(aniso));
}

NX_EXPORT void nxTextureSetRepeating(NxTexture* texture, uint32_t repeating)
{
    texture->setRepeating(repeating);
}

NX_EXPORT void nxTextureSetLessOrEqual(NxTexture* texture, bool enabled)
{
    texture->setLessOrEqual(enabled);
}

NX_EXPORT uint32_t nxTextureFilter(const NxTexture* texture)
{
    return static_cast<uint32_t>(texture->filter());
}

NX_EXPORT uint32_t nxTextureAnisotropyLevel(const NxTexture* texture)
{
    return static_cast<uint32_t>(texture->anisotropyLevel());
}

NX_EXPORT uint32_t nxTextureRepeating(const NxTexture* texture)
{
    return texture->repeating();
}

NX_EXPORT bool nxTextureLessOrEqual(const NxTexture* texture)
{
    return texture->lessOrEqual();
}

NX_EXPORT bool nxTextureFlipCoords(const NxTexture* texture)
{
    return texture->flipCoords();
}

NX_EXPORT uint8_t nxTextureType(const NxTexture* texture)
{
    return static_cast<uint8_t>(texture->type());
}

NX_EXPORT uint8_t nxTextureFormat(const NxTexture* texture)
{
    return static_cast<uint8_t>(texture->format());
}

NX_EXPORT uint32_t nxTextureUsedMemory()
{
    return Texture::usedMemory();
}

NX_EXPORT void nxTextureBind(const NxTexture* texture, uint8_t texSlot)
{
    Texture::bind(texture, texSlot);
}
