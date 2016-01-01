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

#include "image.hpp"
#include "../system/log.hpp"

#include <physfs/physfs.h>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

// stb_image callbacks that operate on a BinaryFile
static int read(void* userdata, char* data, int size)
{
    auto* file = reinterpret_cast<PHYSFS_File*>(userdata);
    auto status = PHYSFS_readBytes(file, data, size);

    return status;
}

static void skip(void* userdata, int size)
{
    auto* file = static_cast<PHYSFS_File*>(userdata);

    auto pos = PHYSFS_tell(file);
    if (pos < 0) PHYSFS_seek(file, 0);
    else         PHYSFS_seek(file, pos + size);
}

static int eof(void* userdata)
{
    auto* file = static_cast<PHYSFS_File*>(userdata);

    auto pos = PHYSFS_tell(file);
    if (pos < 0) return 1;

    auto size = PHYSFS_fileLength(file);
    if (size < 0) return 1;

    return pos >= size;
}

void Image::create(unsigned int width, unsigned int height, uint8_t r, uint8_t g, uint8_t b,
    uint8_t a)
{
    if (width && height) {
        // Assign size
        mWidth = width;
        mHeight = height;

        // Resize the pixel buffer
        mPixels.resize(width * height * 4);

        // Fill it with the specified color
        uint8_t* ptr = &mPixels[0];
        uint8_t* end = ptr + mPixels.size();
        while (ptr < end) {
            *ptr++ = r;
            *ptr++ = g;
            *ptr++ = b;
            *ptr++ = a;
        }
    }
    else {
        mWidth = 0;
        mHeight = 0;
        mPixels.clear();
    }
}

void Image::create(unsigned int width, unsigned int height, const uint8_t* pixels)
{
    if (pixels && width && height) {
        // Assign the new size
        mWidth = width;
        mHeight = height;

        // Copy the pixels
        size_t size = width * height * 4;
        mPixels.resize(size);
        memcpy(&mPixels[0], pixels, size);
    }
    else {
        mWidth = 0;
        mHeight = 0;
        mPixels.clear();
    }
}

bool Image::open(const std::string& filename)
{
    PHYSFS_File* file = PHYSFS_openRead(filename.data());
    if (file) return open(file, true);

    Log::error("Failed to load image \"%s\".", filename.data());
    return false;
}

bool Image::open(const void* data, size_t size)
{
    // Clear the array (just in case)
    mPixels.clear();
    mWidth  = 0u;
    mHeight = 0u;

    if (!data || !size) {
        Log::error("Failed to load image from memory, no data provided");
        return false;
    }

    int width, height, channels;
    auto buffer = reinterpret_cast<const unsigned char*>(data);
    auto bufferSize = static_cast<int>(size);
    unsigned char* ptr = stbi_load_from_memory(buffer, bufferSize, &width, &height, &channels,
        STBI_rgb_alpha);

    if (ptr && width && height) {
        // Assign the image size
        mWidth = width;
        mHeight = height;

        // Copy the loaded pixels to the pixel buffer
        mPixels.resize(width * height * 4);
        memcpy(&mPixels[0], ptr, mPixels.size());

        // Free the loaded pixels (they are now in our own pixel buffer)
        stbi_image_free(ptr);

        return true;
    }

    // Error failed to load the image
    Log::error("Failed to load image from memory. Reason: %s", stbi_failure_reason());

    return false;
}

bool Image::open(PHYSFS_File* file, bool closeFile)
{
    // Clear the array (just in case)
    mPixels.clear();
    mWidth  = 0u;
    mHeight = 0u;

    // Make sure that the stream's reading position is a the beginning
    if (!PHYSFS_seek(file, 0)) {
        Log::error("Failed to load image from file handle");
        return false;
    }

    // Setup the stb_image callbacks
    stbi_io_callbacks callbacks;
    callbacks.read = &read;
    callbacks.skip = &skip;
    callbacks.eof  = &eof;

    // Load the imagee and get a pointer to the pixels in memory
    int width, height, channels;
    unsigned char* ptr = stbi_load_from_callbacks(&callbacks, file, &width, &height, &channels,
        STBI_rgb_alpha);
    if (ptr && width & height) {
        // Assign the image properties
        mWidth = width;
        mHeight = height;

        // Copy the loaded pixels to the pixel buffer
        mPixels.resize(width * height * 4);
        memcpy(&mPixels[0], ptr, mPixels.size());

        // Free the loaded pixels (they are now in our own pixel buffer)
        stbi_image_free(ptr);

        // Close the file if needed
        if (closeFile) PHYSFS_close(file);

        return true;
    }

    // Error, failed to load the image
    Log::error("Failed to load image from file handle. Reason: %s", stbi_failure_reason());
    return false;
}

bool Image::save(const std::string& filename) const
{
    if (!mPixels.empty() && (mWidth > 0) && (mHeight > 0)) {
        int len;
        auto buffer = static_cast<const unsigned char*>(mPixels.data());

        unsigned char* png = stbi_write_png_to_mem(
            const_cast<unsigned char*>(buffer), 0, mWidth, mHeight, 4, &len
        );

        if (png) {
            PHYSFS_File* file = PHYSFS_openWrite(filename.data());

            if (file && PHYSFS_writeBytes(file, png, len) >= 0u) {
                stbi_image_free(png);
                PHYSFS_close(file);
                return true;
            }
            else {
                stbi_image_free(png);
            }
        }
    }

    Log::error("Failed to save PNG file \"%s\"", filename.data());
    return false;
}

void Image::getSize(unsigned int* width, unsigned int* height) const
{
    *width = mWidth;
    *height = mHeight;
}

void Image::createMaskFromColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint8_t targetAlpha)
{
    // Make sure that the image is not empty
    if (mPixels.empty()) return;

    // Replace the alpha of the pixels that match the transparent color
    uint8_t* ptr = &mPixels[0];
    uint8_t* end = ptr + mPixels.size();
    while (ptr < end) {
        if ((ptr[0] == r) && (ptr[1] == g) && (ptr[2] == b) && (ptr[3] == a)) {
            ptr[3] = targetAlpha;
        }

        ptr += 4;
    }
}

void Image::copy(const uint8_t* source, int srcX, int srcY, int stride, int dstX, int dstY,
    int dstW, int dstH, bool applyAlpha)
{
    // Then find the valid bounds of the destination rectangle
    int width  = dstW;
    int height = dstH;
    if (static_cast<uint32_t>(dstX + width)  > mWidth)  width  = mWidth - dstX;
    if (static_cast<uint32_t>(dstY + height) > mHeight) height = mHeight - dstY;

    // Make sure the destination area is valid
    if (width <= 0 || height <= 0) return;

    // Precompute as much as possible
    int pitch     = 4 * width;
    int rows      = height;
    int srcStride = 4 * stride;
    int dstStride = 4 * mWidth;
    auto* srcPixels = &source[0] + 4 * (srcX + srcY * stride);
    auto* dstPixels = &mPixels[0] + 4 * (dstX + dstY * mWidth);

    // Copy th pixels
    if (applyAlpha) {
        // Interpolation using alpha values, pixel by pixel (slow)
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < width; ++j) {
                // Get a direct pointer to the components of the current pixel
                const uint8_t* src = srcPixels + j * 4;
                uint8_t*       dst = dstPixels + j * 4;

                // Interpolate RGBA components using the source alpha
                uint8_t alpha = src[3];
                dst[0] = (src[0] * alpha + dst[0] * (255 - alpha)) / 255;
                dst[1] = (src[1] * alpha + dst[1] * (255 - alpha)) / 255;
                dst[2] = (src[2] * alpha + dst[2] * (255 - alpha)) / 255;
                dst[3] = alpha + dst[3] * (255 - alpha) / 255;
            }

            srcPixels += srcStride;
            dstPixels += dstStride;
        }
    }
    else {
        // Optimized copy ignoring alpha values, row by row (fast)
        for (int i = 0; i < rows; ++i) {
            memcpy(dstPixels, srcPixels, pitch);
            srcPixels += srcStride;
            dstPixels += dstStride;
        }
    }
}

void Image::copy(const Image& source, int srcX, int srcY, int dstX, int dstY, int dstW, int dstH,
    bool applyAlpha)
{
    // Make sure that both images are valid
    if (!source.mWidth || !source.mHeight || !mWidth || !mHeight) return;

    // Adjust the source rectangle
    if (dstW == 0 || dstH == 0) {
        srcX = 0;
        srcY = 0;
        dstW = source.mWidth;
        dstX = source.mHeight;
    }
    else {
        if (dstW + srcX > static_cast<int>(source.mWidth))  dstW = source.mWidth - srcX;
        if (dstH + srcY > static_cast<int>(source.mHeight)) dstH = source.mHeight - srcY;
    }

    copy(source.getPixelsPtr(), srcX, srcY, source.mWidth, dstX, dstY, dstW, dstH, applyAlpha);
}

void Image::setPixel(unsigned int x, unsigned int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint8_t* pixel = &mPixels[(x + y * mWidth) * 4];
    *pixel++ = r;
    *pixel++ = g;
    *pixel++ = b;
    *pixel++ = a;
}

void Image::getPixel(unsigned int x, unsigned int y, uint8_t* r, uint8_t* g, uint8_t* b,
    uint8_t* a) const
{
    const uint8_t* pixel = &mPixels[(x + y * mWidth) * 4];
    *r = *pixel++;
    *g = *pixel++;
    *b = *pixel++;
    *a = *pixel++;
}

const uint8_t* Image::getPixelsPtr() const
{
    return mPixels.data();
}

void Image::flipHorizontally()
{
    if (mPixels.empty()) return;

    size_t rowSize = mWidth * 4;
    for (size_t y = 0; y < mHeight; ++y) {
        auto left  = mPixels.begin() + y * rowSize;
        auto right = mPixels.begin() + (y + 1) * rowSize - 4;

        for (size_t x = 0; x < mWidth / 2; ++x) {
            std::swap_ranges(left, left + 4, right);

            left  += 4;
            right -= 4;
        }
    }
}

void Image::flipVertically()
{
    if (mPixels.empty()) return;

    size_t rowSize = mWidth * 4;
    auto top    = mPixels.begin();
    auto bottom = mPixels.end() - rowSize;

    for (size_t y = 0; y < mHeight / 2; ++y) {
        std::swap_ranges(top, top + rowSize, bottom);

        top    += rowSize;
        bottom -= rowSize;
    }
}
