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

#include "fontstack.hpp"

void FontStack::addFont(const Font& font, bool prepend)
{
    if (prepend) {
        mFonts.insert(mFonts.begin(), &font);
    }
    else {
        mFonts.push_back(&font);
    }
}

void FontStack::addFont(const FontStack& stack, bool prepend)
{
    if (prepend) {
        mFonts.insert(mFonts.begin(), stack.mFonts.begin(), stack.mFonts.end());
    }
    else {
        mFonts.insert(mFonts.end(), stack.mFonts.begin(), stack.mFonts.end());
    }
}

const Glyph& FontStack::glyph(uint32_t codePoint, uint32_t charSize, bool bold) const
{
    uint32_t key = ((bold ? 1 : 0) << 31) | codePoint;

    // Search the glyph from the cache
    auto it = mGlyphs[charSize].find(key);
    if (it != mGlyphs[charSize].end()) {
        // Found: just return it
        return it->second;
    }

    // Not found, cycle through fonts to find one that has the glyph
    Glyph glyph;
    for (uint32_t i = 0; i < mFonts.size(); ++i) {
        glyph = mFonts[i]->glyph(codePoint, charSize, bold);

        // Incorporate the glyph's font index into the page
        glyph.page |= i << 16;

        // If the glyph we found is valid, stop looking
        if (glyph.valid) break;
    }

    // Cache the glyph internally for later use
    return mGlyphs[charSize].emplace(key, glyph).first->second;
}

float FontStack::kerning(uint32_t first, uint32_t second, uint32_t charSize) const
{
    // Make sure that both glyphs belong to the same font
    auto firstFontIndex  = (glyph(first, charSize, false).page >> 16) & 0xFFFFu;
    auto secondFontIndex = (glyph(second, charSize, false).page >> 16) & 0xFFFFu;

    if (firstFontIndex == secondFontIndex) {
        return mFonts[firstFontIndex]->kerning(first, second, charSize);
    }

    // Not found, assume kerning of 0
    return 0.f;
}

float FontStack::lineSpacing(uint32_t charSize) const
{
    // Make sure there's at least one font
    if (mFonts.empty()) return 0;

    return mFonts[0]->lineSpacing(charSize);
}

float FontStack::underlinePosition(uint32_t charSize) const
{
    // Make sure there's at least one font
    if (mFonts.empty()) return 0;

    return mFonts[0]->underlinePosition(charSize);
}

float FontStack::underlineThickness(uint32_t charSize) const
{
    // Make sure there's at least one font
    if (mFonts.empty()) return 0;

    return mFonts[0]->underlineThickness(charSize);
}

const Texture* FontStack::texture(uint32_t charSize, uint32_t index) const
{
    // Deduce font and texture indices
    uint16_t fontIndex = (index >> 16) & 0xFFFF;
    uint16_t texIndex  =  index        & 0xFFFF;

    return mFonts[fontIndex]->texture(charSize, texIndex);
}
