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

//==========================================================
// Represents a VRAM texture
//==========================================================
class Texture
{
public:
    Texture() = default;
    Texture(uint8_t type, uint8_t format, uint32_t handle, uint16_t width, uint16_t height,
        uint16_t depth, uint32_t samplerState, bool rbTexture);
    ~Texture();

    uint8_t create(uint8_t texType, uint8_t format, uint16_t width, uint16_t height, uint16_t depth,
        bool hasMips, bool mipMaps, bool sRGB);
    void setData(const void* buffer, int32_t x, int32_t y, int32_t z, int32_t width,
        int32_t height, int32_t depth, uint8_t slice, uint8_t mipLevel);
    bool data(void* buffer, uint8_t slice, uint8_t mipLevel) const;
    void size(uint16_t& width, uint16_t& height, uint16_t& depth) const;
    uint32_t bufferSize() const;

    void setFilter(uint32_t filter);
    void setAnisotropyLevel(uint32_t aniso);
    void setRepeatingX(uint32_t repeating);
    void setRepeatingY(uint32_t repeating);
    void setRepeatingZ(uint32_t repeating);
    void setLessOrEqual(bool enable);

    uint32_t filter() const;
    uint32_t anisotropyLevel() const;
    void repeating(uint32_t& x, uint32_t& y, uint32_t& z) const;
    bool lessOrEqual() const;

    uint32_t nativeHandle() const;

    bool flipCoords() const;
    uint8_t texType() const;
    uint8_t texFormat() const;

    static uint32_t calcSize(uint8_t format, uint16_t width, uint16_t height, uint16_t depth);
    static uint32_t usedMemory();
    static void bind(const Texture* texture, uint8_t texSlot);

private:
    uint8_t  mType         {0u};
    uint8_t  mFormat       {0u};
    uint32_t mHandle       {0u};
    uint16_t mWidth        {0u};
    uint16_t mHeight       {0u};
    uint16_t mDepth        {0u};
    uint32_t mSamplerState {0u};
    bool     mRbTexture    {0u};
};
