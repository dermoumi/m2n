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
#include "../graphics/image.hpp"

//==========================================================
// Declarations
//==========================================================
using NxImage = Image;
struct PHYSFS_File;

//==========================================================
// A set of functions to create and manage images
//==========================================================
NX_EXPORT NxImage* nxImageNew()
{
    return new NxImage();
}

//----------------------------------------------------------
NX_EXPORT void nxImageRelease(NxImage* image)
{
    delete image;
}

//----------------------------------------------------------
NX_EXPORT void nxImageCreateFill(NxImage* image, uint32_t width, uint32_t height, uint8_t r,
    uint8_t g, uint8_t b, uint8_t a)
{
    image->create(width, height, r, g, b, a);
}

//----------------------------------------------------------
NX_EXPORT void nxImageCreateFromData(NxImage* image, uint32_t width, uint32_t height,
    const uint8_t* buffer)
{
    image->create(width, height, buffer);
}

//----------------------------------------------------------
NX_EXPORT bool nxImageOpenFromFile(NxImage* image, const char* filename)
{
    return image->open(filename);
}

//----------------------------------------------------------
NX_EXPORT bool nxImageOpenFromMemory(NxImage* image, const void* buffer, size_t size)
{
    return image->open(buffer, size);
}

//----------------------------------------------------------
NX_EXPORT bool nxImageOpenFromHandle(NxImage* image, PHYSFS_File* file, bool close)
{
    return image->open(file, close);
}

//----------------------------------------------------------
NX_EXPORT bool nxImageSave(const NxImage* image, const char* filename)
{
    return image->save(filename);
}

//----------------------------------------------------------
NX_EXPORT void nxImageGetSize(const NxImage* image, uint32_t* sizePtr)
{
    image->getSize(&sizePtr[0], &sizePtr[1]);
}

//----------------------------------------------------------
NX_EXPORT void nxImageColorMask(NxImage* image, uint8_t r, uint8_t g, uint8_t b, uint8_t a,
    uint8_t alpha)
{
    image->createMaskFromColor(r, g, b, a, alpha);
}

//----------------------------------------------------------
NX_EXPORT void nxImageCopyPixels(NxImage* image, const uint8_t* source, int srcX, int srcY,
    int stride, int dstX, int dstY, int dstW, int dstH, bool applyAlpha)
{
    image->copy(source, srcX, srcY, stride, dstX, dstY, dstW, dstH, applyAlpha);
}

//----------------------------------------------------------
NX_EXPORT void nxImageCopy(NxImage* image, const NxImage* source, int srcX, int srcY, int dstX,
    int dstY, int dstW, int dstH, bool applyAlpha)
{
    image->copy(*source, srcX, srcY, dstX, dstY, dstW, dstH, applyAlpha);
}

//----------------------------------------------------------
NX_EXPORT void nxImageSetPixel(NxImage* image, uint32_t x, uint32_t y, uint8_t r, uint8_t g,
    uint8_t b, uint8_t a)
{
    image->setPixel(x, y, r, g, b, a);
}

//----------------------------------------------------------
NX_EXPORT void nxImageGetPixel(const NxImage* image, uint32_t x, uint32_t y, uint8_t* colPtr)
{
    image->getPixel(x, y, &colPtr[0], &colPtr[1], &colPtr[2], &colPtr[3]);
}

//----------------------------------------------------------
NX_EXPORT const uint8_t* nxImageGetPixelsPtr(const NxImage* image)
{
    return image->getPixelsPtr();
}

//----------------------------------------------------------
NX_EXPORT void nxImageFlipHorizontally(NxImage* image)
{
    image->flipHorizontally();
}

//----------------------------------------------------------
NX_EXPORT void nxImageFlipVertically(NxImage* image)
{
    image->flipVertically();
}

//==============================================================================
