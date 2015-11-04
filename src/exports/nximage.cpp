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
NX_EXPORT NxImage* nxImageCreateFill(unsigned int width, unsigned int height, const uint8_t* color)
{
    NxImage* image = new NxImage();
    image->create(width, height, color[0], color[1], color[2], color[4]);
    return image;
}

//----------------------------------------------------------
NX_EXPORT NxImage* nxImageCreateFromData(unsigned int width, unsigned int height,
    const uint8_t* buffer)
{
    NxImage* image = new NxImage();
    image->create(width, height, buffer);
    return image;
}

//----------------------------------------------------------
NX_EXPORT NxImage* nxImageCreateFromFile(const char* filename)
{
    NxImage* image = new NxImage();
    if (image->open(filename)) return image;

    delete image;
    return nullptr;
}

//----------------------------------------------------------
NX_EXPORT NxImage* nxImageCreateFromMemory(const void* buffer, size_t size)
{
    NxImage* image = new NxImage();
    if (image->open(buffer, size)) return image;
    
    delete image;
    return nullptr;
}

//----------------------------------------------------------
NX_EXPORT NxImage* nxImageCreateFromHandle(PHYSFS_File* file, bool close)
{
    NxImage* image = new NxImage();
    if (image->open(file, close)) return image;

    delete image;
    return nullptr;
}

//----------------------------------------------------------
NX_EXPORT bool nxImageSave(const NxImage* image, const char* filename)
{
    return image->save(filename);
}

//----------------------------------------------------------
NX_EXPORT void nxImageRelease(NxImage* image)
{
    delete image;
}

//==============================================================================
