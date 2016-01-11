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

#include "renderbuffer.hpp"
#include "renderdevice.hpp"

Renderbuffer::~Renderbuffer()
{
    if (mHandle) {
        RenderDevice::instance().destroyRenderBuffer(mHandle);
    }
}

uint8_t Renderbuffer::create(uint8_t format, uint16_t width, uint16_t height, bool depth,
    uint8_t colBufCount, uint8_t samples)
{
    if (width == 0 || height == 0) return 1;
    uint32_t handle = RenderDevice::instance().createRenderBuffer(
        width, height, static_cast<RenderDevice::TextureFormat>(format), depth, colBufCount, samples
    );

    if (!handle) return 2;

    if (mHandle) {
        RenderDevice::instance().destroyRenderBuffer(mHandle);
    }

    mFormat           = format;
    mHandle           = handle;
    mWidth            = width;
    mHeight           = height;
    mDepth            = depth;
    mColorBufferCount = colBufCount;
    mSamples          = samples;

    return 0;
}

#include "../system/log.hpp"

Texture* Renderbuffer::texture(uint8_t bufIndex)
{
    Texture* tex {nullptr};

    if (bufIndex == 32 && mDepth) {
        tex = mTextures[4].get();

        if (!tex) {
            mTextures[4] = std::unique_ptr<Texture>(new Texture(
                RenderDevice::Tex2D, RenderDevice::DEPTH,
                RenderDevice::instance().getRenderBufferTexture(mHandle, 32),
                mWidth, mHeight, 1, 0, true
            ));
            tex = mTextures[4].get();
        }
    }
    else if (bufIndex < mColorBufferCount) {
        tex = mTextures[bufIndex].get();

        if (!tex) {
            mTextures[bufIndex] = std::unique_ptr<Texture>(new Texture(
                RenderDevice::Tex2D, mFormat,
                RenderDevice::instance().getRenderBufferTexture(mHandle, bufIndex),
                mWidth, mHeight, 1, 0, true
            ));
            tex = mTextures[bufIndex].get();
        }
    }

    return tex;
}

void Renderbuffer::size(uint16_t& width, uint16_t& height) const
{
    width = mWidth;
    height = mHeight;
}

uint8_t Renderbuffer::texFormat() const
{
    return mFormat;
}

void Renderbuffer::bind(const Renderbuffer* buffer)
{
    RenderDevice::instance().setRenderBuffer(buffer ? buffer->mHandle : 0);
}
