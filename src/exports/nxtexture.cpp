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
#include "../graphics/texture.hpp"

using NxTexture = Texture;

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT NxTexture* nxTextureNew()
{
    return new Texture();
}

//----------------------------------------------------------
NX_EXPORT NxTexture* nxTextureFromData(uint8_t type, uint8_t format, uint32_t handle,
    uint16_t width, uint16_t height, uint16_t depth, uint32_t samplerState, bool rbTexture)
{
    return new Texture(type, format, handle, width, height, depth, samplerState, rbTexture);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureRelease(NxTexture* texture)
{
    delete texture;
}

//----------------------------------------------------------
NX_EXPORT uint8_t nxTextureCreate(NxTexture* texture, uint8_t type, uint8_t format, uint16_t width,
    uint16_t height, uint16_t depth, bool hasMips, bool mipMaps, bool sRGB)
{
    return texture->create(type, format, width, height, depth, hasMips, mipMaps, sRGB);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureSetData(NxTexture* texture, const void* buffer, int32_t x, int32_t y,
    int32_t z, int32_t width, int32_t height, int32_t depth, uint8_t slice, uint8_t mipLevel)
{
    texture->setData(buffer, x, y, z, width, height, depth, slice, mipLevel);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureBind(NxTexture* texture, uint8_t texSlot)
{
    texture->bind(texSlot);
}

//----------------------------------------------------------
NX_EXPORT bool nxTextureData(const NxTexture* texture, void* buffer, uint8_t slice,
    uint8_t mipLevel)
{
    return texture->data(buffer, slice, mipLevel);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureSize(const NxTexture* texture, uint16_t* sizePtr)
{
    texture->size(sizePtr[0], sizePtr[1], sizePtr[2]);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxTextureBufferSize(const NxTexture* texture)
{
    return texture->bufferSize();
}

//----------------------------------------------------------
NX_EXPORT void nxTextureSetFilter(NxTexture* texture, uint32_t filter)
{
    texture->setFilter(filter);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureSetAnisotropyLevel(NxTexture* texture, uint32_t aniso)
{
    texture->setAnisotropyLevel(aniso);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureSetRepeatingX(NxTexture* texture, uint32_t repeating)
{
    texture->setRepeatingX(repeating);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureSetRepeatingY(NxTexture* texture, uint32_t repeating)
{
    texture->setRepeatingY(repeating);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureSetRepeatingZ(NxTexture* texture, uint32_t repeating)
{
    texture->setRepeatingZ(repeating);
}

//----------------------------------------------------------
NX_EXPORT void nxTextureSetLessOrEqual(NxTexture* texture, bool enabled)
{
    texture->setLessOrEqual(enabled);
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxTextureFilter(const NxTexture* texture)
{
    return texture->filter();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxTextureAnisotropyLevel(const NxTexture* texture)
{
    return texture->anisotropyLevel();
}

//----------------------------------------------------------
NX_EXPORT void nxTextureRepeating(const NxTexture* texture, uint32_t* repeatingPtr)
{
    texture->repeating(repeatingPtr[0], repeatingPtr[1], repeatingPtr[2]);
}

//----------------------------------------------------------
NX_EXPORT bool nxTextureLessOrEqual(const NxTexture* texture)
{
    return texture->lessOrEqual();
}

//----------------------------------------------------------
NX_EXPORT bool nxTextureFlipCoords(const NxTexture* texture)
{
    return texture->flipCoords();
}

//----------------------------------------------------------
NX_EXPORT uint8_t nxTextureType(const NxTexture* texture)
{
    return texture->texType();
}

//----------------------------------------------------------
NX_EXPORT uint8_t nxTextureFormat(const NxTexture* texture)
{
    return texture->texFormat();
}

//----------------------------------------------------------
NX_EXPORT uint32_t nxTextureUsedMemory()
{
    return Texture::usedMemory();
}
