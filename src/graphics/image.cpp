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
#include "image.hpp"
#include "../system/log.hpp"

#include <physfs/physfs.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>

//==========================================================
// Locals
//==========================================================
namespace
{
    // stb_image callbacks that operate on a BinaryFile
    int read(void* userdata, char* data, int size)
    {
        auto* file = reinterpret_cast<PHYSFS_File*>(userdata);
        auto status = PHYSFS_readBytes(file, data, size);

        return status;
    }

    void skip(void* userdata, int size)
    {
        auto* file = static_cast<PHYSFS_File*>(userdata);

        auto pos = PHYSFS_tell(file);
        if (pos < 0) PHYSFS_seek(file, 0);
        else         PHYSFS_seek(file, pos + size);
    }

    int eof(void* userdata)
    {
        auto* file = static_cast<PHYSFS_File*>(userdata);

        auto pos = PHYSFS_tell(file);
        if (pos < 0) return 1;

        auto size = PHYSFS_fileLength(file);
        if (size < 0) return 1;

        return pos >= size;
    }
}

//----------------------------------------------------------
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

//----------------------------------------------------------
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

//----------------------------------------------------------
bool Image::open(const std::string& filename)
{
    PHYSFS_File* file = PHYSFS_openRead(filename.data());
    if (file) return open(file, true);

    Log::error("Failed to load image \"%s\".", filename.data());
    return false;
}

//----------------------------------------------------------
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

//----------------------------------------------------------
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

//----------------------------------------------------------
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

//----------------------------------------------------------
void Image::getSize(unsigned int* width, unsigned int* height) const
{
    *width = mWidth;
    *height = mHeight;
}

//----------------------------------------------------------
const uint8_t* Image::getPixelsPtr() const
{
    return mPixels.data();
}

//==============================================================================
