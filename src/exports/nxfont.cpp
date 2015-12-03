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
#include "../graphics/font.hpp"

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxFont    = Font;
using NxTexture = Texture;

//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT void nxFontRelease(NxFont* font)
{
    delete font;
}

//----------------------------------------------------------
NX_EXPORT void nxFontGlyph(const NxFont* font, uint32_t codePoint, uint32_t charSize,
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
NX_EXPORT float nxFontKerning(const NxFont* font, uint32_t first, uint32_t second,
    uint32_t charSize)
{
    return font->kerning(first, second, charSize);
}

//----------------------------------------------------------
NX_EXPORT float nxFontLineSpacing(const NxFont* font, uint32_t charSize)
{
    return font->lineSpacing(charSize);
}

//----------------------------------------------------------
NX_EXPORT float nxFontUnderlinePosition(const NxFont* font, uint32_t charSize)
{
    return font->underlinePosition(charSize);
}

//----------------------------------------------------------
NX_EXPORT float nxFontUnderlineThickness(const NxFont* font, uint32_t charSize)
{
    return font->underlineThickness(charSize);
}

//----------------------------------------------------------
NX_EXPORT const NxTexture* nxFontTexture(const NxFont* font, uint32_t charSize, uint32_t index)
{
    return font->texture(charSize, index);
}