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
#include "texture.hpp"
#include "renderdevice.hpp"

//----------------------------------------------------------
static uint16_t maximumSize()
{
    unsigned int maxSize;
    RenderDevice::instance().getCapabilities(nullptr, &maxSize, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    return maxSize;
}

//----------------------------------------------------------
Texture::Texture(uint8_t type, uint8_t format, uint32_t handle, uint16_t width, uint16_t height,
    uint16_t depth, uint32_t samplerState, bool rbTexture) :
    mType(type), mFormat(format), mHandle(handle), mWidth(width), mHeight(height), mDepth(depth),
    mSamplerState(samplerState), mRbTexture(rbTexture)
{
    // Nothingh else to do
}

//----------------------------------------------------------
Texture::~Texture()
{
    if (mHandle && !mRbTexture) {
        RenderDevice::instance().destroyTexture(mHandle);
    }
}

//----------------------------------------------------------
uint8_t Texture::create(uint8_t type, uint8_t format, uint16_t width, uint16_t height,
    uint16_t depth, bool hasMips, bool mipMaps, bool sRGB)
{
    // Make sure that the size is valid
    if (width == 0 || height == 0 || depth == 0) {
        return 1;
    }

    // Check maximum size
    static uint16_t maxSize = maximumSize();
    if (width > maxSize || height > maxSize || depth > maxSize) {
        return 2;
    }

    // Destroy current texture if any
    if (mHandle) {
        RenderDevice::instance().destroyTexture(mHandle);
    }

    uint32_t handle = RenderDevice::instance().createTexture(
        static_cast<RenderDevice::TextureType>(type), width, height, depth,
        static_cast<RenderDevice::TextureFormat>(format), hasMips, mipMaps, sRGB
    );
    if (!handle) {
        return 3;
    }

    mHandle       = handle;
    mType         = type;
    mFormat       = format;
    mWidth        = width;
    mHeight       = height;
    mDepth        = depth;
    mSamplerState = 0u;
    mRbTexture    = false;

    return 0;
}

//----------------------------------------------------------
void Texture::setData(const void* buffer, int32_t x, int32_t y, int32_t z, int32_t width,
    int32_t height, int32_t depth, uint8_t slice, uint8_t mipLevel)
{
    if (x < 0) {
        RenderDevice::instance().uploadTextureData(mHandle, slice, mipLevel, buffer);
    }
    else {
        RenderDevice::instance().uploadTextureSubData(
            mHandle, slice, mipLevel, x, y, z, width, height, depth, buffer
        );
    }
}

//----------------------------------------------------------
bool Texture::data(void* buffer, uint8_t slice, uint8_t mipLevel) const
{
    return RenderDevice::instance().getTextureData(mHandle, slice, mipLevel, buffer);
}

//----------------------------------------------------------
void Texture::size(uint16_t& width, uint16_t& height, uint16_t& depth) const
{
    width = mWidth;
    height = mHeight;
    depth = mDepth;
}

//----------------------------------------------------------
uint32_t Texture::bufferSize() const
{
    return calcSize(mFormat, mWidth, mHeight, mDepth);
}

//----------------------------------------------------------
void Texture::setFilter(uint32_t filter)
{
    mSamplerState &= ~RenderDevice::FilterMask;
    mSamplerState |= filter;
}

//----------------------------------------------------------
void Texture::setAnisotropyLevel(uint32_t aniso)
{
    mSamplerState &= ~RenderDevice::AnisoMask;
    mSamplerState |= aniso;
}

//----------------------------------------------------------
void Texture::setRepeatingX(uint32_t repeating)
{
    mSamplerState &= ~RenderDevice::AddrUMask;
    mSamplerState |= repeating;
}

//----------------------------------------------------------
void Texture::setRepeatingY(uint32_t repeating)
{
    mSamplerState &= ~RenderDevice::AddrVMask;
    mSamplerState |= repeating;
}

//----------------------------------------------------------
void Texture::setRepeatingZ(uint32_t repeating)
{
    mSamplerState &= ~RenderDevice::AddrWMask;
    mSamplerState |= repeating;
}

//----------------------------------------------------------
void Texture::setLessOrEqual(bool enabled)
{
    if (enabled) {
        mSamplerState |= RenderDevice::CompLEqual;
    }
    else {
        mSamplerState &= ~RenderDevice::CompLEqual;
    }
}

//----------------------------------------------------------
uint32_t Texture::filter() const
{
    return mSamplerState & RenderDevice::FilterMask;
}

//----------------------------------------------------------
uint32_t Texture::anisotropyLevel() const
{
    return mSamplerState & RenderDevice::AnisoMask;
}

//----------------------------------------------------------
void Texture::repeating(uint32_t& x, uint32_t& y, uint32_t& z) const
{
    x = mSamplerState & RenderDevice::AddrUMask;
    y = mSamplerState & RenderDevice::AddrVMask;
    z = mSamplerState & RenderDevice::AddrWMask;
}

//----------------------------------------------------------
bool Texture::lessOrEqual() const
{
    return mSamplerState & RenderDevice::CompLEqual;
}

//----------------------------------------------------------
uint32_t Texture::nativeHandle() const
{
    return mHandle;
}

//----------------------------------------------------------
bool Texture::flipCoords() const
{
    return mRbTexture;
}

//----------------------------------------------------------
uint8_t Texture::texType() const
{
    return mType;
}

//----------------------------------------------------------
uint8_t Texture::texFormat() const
{
    return mFormat;
}

//----------------------------------------------------------
uint32_t Texture::calcSize(uint8_t format, uint16_t width, uint16_t height, uint16_t depth)
{
    return RenderDevice::instance().calcTextureSize(
        static_cast<RenderDevice::TextureFormat>(format), width, height, depth
    );
}

//----------------------------------------------------------
uint32_t Texture::usedMemory()
{
    return RenderDevice::instance().getTextureMemory();
}

//----------------------------------------------------------
void Texture::bind(const Texture* texture, uint8_t texSlot)
{
    if (!texture) {
        RenderDevice::instance().setTexture(texSlot, 0, 0);
    }
    else {
        RenderDevice::instance().setTexture(texSlot, texture->mHandle, texture->mSamplerState);
    }
}
