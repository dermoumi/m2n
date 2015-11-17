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
#include "../graphics/vectorfont.hpp"

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxVectorFont = VectorFont;
using NxTexture    = Texture;
struct PHYSFS_File;

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT NxVectorFont* nxVectorFontNew()
{
    return new NxVectorFont();
}

//----------------------------------------------------------
NX_EXPORT void nxVectorFontRelease(NxVectorFont* font)
{
    delete font;
}

//----------------------------------------------------------
NX_EXPORT bool nxVectorFontOpenFromFile(NxVectorFont* font, const char* filename)
{
    return font->open(filename);
}

//----------------------------------------------------------
NX_EXPORT bool nxVectorFontOpenFromMemory(NxVectorFont* font, const void* buffer, size_t size)
{
    return font->open(buffer, size);
}

//----------------------------------------------------------
NX_EXPORT bool nxVectorFontOpenFromHandle(NxVectorFont* font, PHYSFS_File* file)
{
    return font->open(file);
}

//----------------------------------------------------------
NX_EXPORT const char* nxVectorFontFamilyName(const NxVectorFont* font)
{
    return font->info().family.data();
}

//----------------------------------------------------------
NX_EXPORT void nxVectorFontGlyph(const NxVectorFont* font, uint32_t codePoint, uint32_t charSize,
    bool bold, float* fValues, uint32_t* uValues)
{
    auto& glyph = font->glyph(codePoint, charSize, bold);
    fValues[0] = glyph.advance;
    fValues[1] = glyph.left;
    fValues[2] = glyph.top;
    fValues[3] = glyph.width;
    fValues[4] = glyph.height;
    uValues[0] = glyph.texLeft;
    uValues[1] = glyph.texTop;
    uValues[2] = glyph.texWidth;
    uValues[3] = glyph.texHeight;
}

//----------------------------------------------------------
NX_EXPORT float nxVectorFontKerning(const NxVectorFont* font, uint32_t first, uint32_t second,
    uint32_t charSize)
{
    return font->kerning(first, second, charSize);
}

//----------------------------------------------------------
NX_EXPORT float nxVectorFontLineSpacing(const NxVectorFont* font, uint32_t charSize)
{
    return font->lineSpacing(charSize);
}

//----------------------------------------------------------
NX_EXPORT float nxVectorFontUnderlinePosition(const NxVectorFont* font, uint32_t charSize)
{
    return font->underlinePosition(charSize);
}

//----------------------------------------------------------
NX_EXPORT float nxVectorFontUnderlineThickness(const NxVectorFont* font, uint32_t charSize)
{
    return font->underlineThickness(charSize);
}

//----------------------------------------------------------
NX_EXPORT const NxTexture* nxVectorFontTexture(const NxVectorFont* font, uint32_t charSize)
{
    return &font->texture(charSize);
}