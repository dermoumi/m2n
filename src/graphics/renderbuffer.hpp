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
#include "texture.hpp"

#include <memory>

//==========================================================
// Represents a drawable texture in the VRAM
//==========================================================
class Renderbuffer
{
public:
    Renderbuffer() = default;
    ~Renderbuffer();

    uint8_t create(uint8_t format, uint16_t width, uint16_t height, bool depth, uint8_t colBufCount,
        uint8_t samples);
    Texture* texture(uint8_t bufIndex);

    void size(uint16_t& width, uint16_t& height) const;
    uint8_t texFormat() const;

    static void setCurrent(const Renderbuffer* buffer);

private:
    uint8_t  mFormat           {0u};
    uint32_t mHandle           {0u};
    uint16_t mWidth            {0u};
    uint16_t mHeight           {0u};
    bool     mDepth            {false};
    uint8_t  mColorBufferCount {0u};
    uint8_t  mSamples          {0u};

    std::unique_ptr<Texture> mTextures[5];
};