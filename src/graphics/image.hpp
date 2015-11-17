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

#include <string>
#include <vector>

//==========================================================
// Declarations
//==========================================================
struct PHYSFS_File;

//==========================================================
// A class to load and handle images
//==========================================================
class Image
{
public:
    Image() = default;

    void create(unsigned int width, unsigned int height,
        uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void create(unsigned int width, unsigned int height, const uint8_t* pixels);

    bool open(const std::string& filename);
    bool open(const void* data, size_t size);
    bool open(PHYSFS_File* file, bool closeFile);

    bool save(const std::string& filename) const;

    void getSize(unsigned int* width, unsigned int* height) const;
    void createMaskFromColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a, uint8_t targetAlpha = 0);
    void copy(const uint8_t* source, int srcX, int srcY, int stride, int dstX, int dstY,
        int dstW, int dstH, bool applyAlpha);
    void copy(const Image& source, int srcX, int srcY, int dstX, int dstY, int dstW, int dstH,
        bool applyAlpha);
    void setPixel(unsigned int x, unsigned int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    void getPixel(unsigned int x, unsigned int y, uint8_t* r, uint8_t* g, uint8_t* b,
        uint8_t* a) const;
    
    const uint8_t* getPixelsPtr() const;
    void flipHorizontally();
    void flipVertically();

private:
    unsigned int mWidth;
    unsigned int mHeight;
    std::vector<uint8_t> mPixels;
};

//==============================================================================
