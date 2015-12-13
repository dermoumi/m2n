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
#include "text.hpp"
#include "../system/unicode.hpp"

#include <cmath>
#include <set>

//----------------------------------------------------------
static bool isHarakat(uint32_t haraka)
{
    static std::set<uint32_t> harakat {
        0x0600, 0x0601, 0x0602, 0x0603, 0x0606, 0x0607, 0x0608, 0x0609,
        0x060A, 0x060B, 0x060D, 0x060E, 0x0610, 0x0611, 0x0612, 0x0613,
        0x0614, 0x0615, 0x0616, 0x0617, 0x0618, 0x0619, 0x061A, 0x061B,
        0x061E, 0x061F, 0x0621, 0x063B, 0x063C, 0x063D, 0x063E, 0x063F,
        0x0640, 0x064B, 0x064C, 0x064D, 0x064E, 0x064F, 0x0650, 0x0651,
        0x0652, 0x0653, 0x0654, 0x0655, 0x0656, 0x0657, 0x0658, 0x0659,
        0x065A, 0x065B, 0x065C, 0x065D, 0x065E, 0x0660, 0x066A, 0x066B,
        0x066C, 0x066F, 0x0670, 0x0672, 0x06D4, 0x06D5, 0x06D6, 0x06D7,
        0x06D8, 0x06D9, 0x06DA, 0x06DB, 0x06DC, 0x06DF, 0x06E0, 0x06E1,
        0x06E2, 0x06E3, 0x06E4, 0x06E5, 0x06E6, 0x06E7, 0x06E8, 0x06E9,
        0x06EA, 0x06EB, 0x06EC, 0x06ED, 0x06EE, 0x06EF, 0x06D6, 0x06D7,
        0x06D8, 0x06D9, 0x06DA, 0x06DB, 0x06DC, 0x06DD, 0x06DE, 0x06DF,
        0x06F0, 0x06FD, 0xFE70, 0xFE71, 0xFE72, 0xFE73, 0xFE74, 0xFE75,
        0xFE76, 0xFE77, 0xFE78, 0xFE79, 0xFE7A, 0xFE7B, 0xFE7C, 0xFE7D,
        0xFE7E, 0xFE7F, 0xFC5E, 0xFC5F, 0xFC60, 0xFC61, 0xFC62, 0xFC63
    };

    return harakat.find(haraka) != harakat.end();
}

//----------------------------------------------------------
void Text::setString(const std::string& str)
{
    setString(Unicode::utf8To32(str));
}

//----------------------------------------------------------
void Text::setString(const std::u32string& str)
{
    mString = str;
    mNeedsUpdate = true;
}

//----------------------------------------------------------
void Text::setFont(const Font& font)
{
    if (mFont == &font) return;

    mFont = &font;
    mNeedsUpdate = true;
}

//----------------------------------------------------------
void Text::setCharacterSize(uint32_t charSize)
{
    if (mCharSize == charSize) return;

    mCharSize = charSize;
    mNeedsUpdate = true;
}

//----------------------------------------------------------
void Text::setStyle(uint8_t style)
{
    if (mStyle == style) return;

    mStyle = style;
    mNeedsUpdate = true;
}

//----------------------------------------------------------
void Text::setRightToLeft(bool rtl)
{
    if (mRightToLeft == rtl) return;

    mRightToLeft = rtl;
    mNeedsUpdate = true;
}

//----------------------------------------------------------
const std::u32string& Text::string() const
{
    return mString;
}

//----------------------------------------------------------
const Font* Text::font() const
{
    return mFont;
}

//----------------------------------------------------------
uint32_t Text::characterSize() const
{
    return mCharSize;
}

//----------------------------------------------------------
uint8_t Text::style() const
{
    return mStyle;
}

//----------------------------------------------------------
bool Text::rightToLeft() const
{
    return mRightToLeft;
}

//----------------------------------------------------------
void Text::characterPosition(size_t index, float& x, float& y) const
{
    // Initialize the positions to 0
    x = y = 0.f;

    // Make sure that we have a valid font
    if (!mFont) return;

    // Adjust the index if it's out of range
    if (index > mString.size()) index = mString.size();

    // Precompute stuff
    bool bold = (mStyle & Bold) != 0;
    float hspace = static_cast<float>(mFont->glyph(U' ', mCharSize, bold).advance);
    float vspace = static_cast<float>(mFont->lineSpacing(mCharSize));

    // Compute the position
    uint32_t prevChar {0u};
    for (size_t i = 0; i < index; ++i) {
        uint32_t currChar = mString[i];

        // Apply the kerning offset
        x += static_cast<float>(mFont->kerning(prevChar, currChar, mCharSize));
        prevChar = currChar;

        // Handle special characters
        switch (currChar) {
            case U' ':  x += hspace;        continue;
            case U'\t': x += hspace * 4;    continue;
            case U'\n': x = 0; y += vspace; continue;
        }

        // For regular characters, add the advance offset of the glyph
        x += static_cast<float>(mFont->glyph(currChar, mCharSize, bold).advance);
    }
}

//----------------------------------------------------------
void Text::bounds(float& x, float& y, float& w, float& h) const
{
    ensureGeometryUpdate();
    x = mBoundsX;
    y = mBoundsY;
    w = mBoundsW;
    h = mBoundsH;
}

//----------------------------------------------------------
const Arraybuffer* Text::arraybuffer(uint32_t& vertexCount, uint32_t index) const
{
    ensureGeometryUpdate();
    vertexCount = mVertices[index].count;
    return &mVertices[index].buffer;
}

//----------------------------------------------------------
uint32_t* Text::arraybufferIDs(uint32_t* count) const
{
    ensureGeometryUpdate();

    *count = mBufferIDs.size();
    return mBufferIDs.data();
}

//----------------------------------------------------------
void Text::ensureGeometryUpdate() const
{
    // If geometry is already up-to-date, do nothing
    if (!mNeedsUpdate) return;

    // Mark the geometry as updated
    mNeedsUpdate = false;

    // Reset variables
    mBoundsX = mBoundsY = mBoundsW = mBoundsH = 0;

    // No font or no string: nothing to draw
    if (!mFont || mString.empty()) {
        return;
    }

    // Temporary buffer
    using BufType = std::vector<float>;
    std::map<uint32_t, BufType> buffers;

    // Compute values related to the text style
    bool bold                = (mStyle & Bold) != 0;
    bool underlined          = (mStyle & Underlined) != 0;
    bool strikeThrough       = (mStyle & StrikeThrough) != 0;
    float italic             = (mStyle & Italic) ? 0.208f : 0.f; // 12 degrees
    float underlineOffset    = mFont->underlinePosition(mCharSize);
    float underlineThickness = mFont->underlineThickness(mCharSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as thee reference
    // We reuse teh underline thickness as the thickness of the strike through as well
    auto& xGlyph = mFont->glyph(U'x', mCharSize, bold);
    float xBoundsX {xGlyph.left}, xBoundsH {xGlyph.height};

    float strikeThroughOffset = xBoundsX + xBoundsH / 2.f;

    // Precompute the variables needed by the algorithm
    float hspace = static_cast<float>(mFont->glyph(U' ', mCharSize, bold).advance);
    float vspace = static_cast<float>(mFont->lineSpacing(mCharSize));
    float x      = 0.f;
    float y      = static_cast<float>(mCharSize);

    float xFactor = mRightToLeft ? -1.f : 1.f;

    // Create create two triangles for each character
    float minX = static_cast<float>(mCharSize);
    float minY = static_cast<float>(mCharSize);
    float maxX = 0.f;
    float maxY = 0.f;
    uint32_t prevChar = 0u;

    for (size_t i = 0u; i < mString.size(); ++i) {
        uint32_t currChar = static_cast<uint32_t>(mString[i]);

        // Apply the kerning offset
        if (mRightToLeft) {
            x += static_cast<float>(mFont->kerning(currChar, prevChar, mCharSize));
        }
        else {
            x += static_cast<float>(mFont->kerning(prevChar, currChar, mCharSize));
        }

        // If we're using the underlined style and there's a new line, draw a line
        if (underlined && (currChar == U'\n')) {
            float top = std::floor(y + underlineOffset - (underlineThickness / 2) + 0.5f);
            float bottom = top + std::floor(underlineThickness + 0.5f);

            float vertices[24] {
                x,   top,    1.f, 1.f,
                0.f, top,    1.f, 1.f,
                0.f, bottom, 1.f, 1.f,
                x,   top,    1.f, 1.f,
                0.f, bottom, 1.f, 1.f,
                x,   bottom, 1.f, 1.f
            };
            buffers[0].insert(buffers[0].end(), std::begin(vertices), std::end(vertices));
        }

        // If we're using strike through style and there's a new line, draw a line accross
        // all characters
        if (strikeThrough && (currChar == U'\n')) {
            float top = std::floor(y + strikeThroughOffset - (underlineThickness / 2) + 0.5f);
            float bottom = top + std::floor(underlineThickness + 0.5f);

            float vertices[24] {
                x,   top,    1.f, 1.f,
                0.f, top,    1.f, 1.f,
                0.f, bottom, 1.f, 1.f,
                x,   top,    1.f, 1.f,
                0.f, bottom, 1.f, 1.f,
                x,   bottom, 1.f, 1.f
            };
            buffers[0].insert(buffers[0].end(), std::begin(vertices), std::end(vertices));
        }

        // Handle special characters
        if (currChar == U' ' || currChar == U'\t' || currChar == U'\n') {
            // Update the current bounds (min coodinates)
            minX = std::min(minX, x);
            minY = std::min(minY, y);

            switch (currChar) {
                case U' ':  x += hspace;        break;
                case U'\t': x += hspace * 4;    break;
                case U'\n': y += vspace; x = 0; break;
            }

            // Update the current bounds (max coordinates)
            maxX = std::max(maxX, x);
            maxY = std::max(maxY, y);

            // next glyph, no need to create a quad for whitespace
            continue;
        }

        // Extract the current glyph's descripts
        const auto& glyph = mFont->glyph(currChar, mCharSize, bold);

        float left   = glyph.left - 0.25f;
        float top    = glyph.top - 0.25f;
        float right  = glyph.left + glyph.width + 0.25f;
        float bottom = glyph.top + glyph.height + 0.25f;

        if (glyph.texWidth != 0 && glyph.texHeight != 0) {
            float u1 = glyph.texLeft - 0.25f;
            float v1 = glyph.texTop - 0.25f;
            float u2 = glyph.texLeft + glyph.texWidth + 0.25f;
            float v2 = glyph.texTop + glyph.texHeight + 0.25f;

            float x1 = x + left;
            float x2 = x + right;
            float y1 = y + top;
            float y2 = y + bottom;

            if (mRightToLeft) {
                x1 = x * xFactor - right + left;
                x2 = x * xFactor;
                if (prevChar && isHarakat(currChar)) {
                    const auto& prevGlyph = mFont->glyph(prevChar, mCharSize, bold);
                    auto xOffset = (prevGlyph.advance + glyph.width) / 2.f;
                    x1 += xOffset;
                    x2 += xOffset;

                    x -= glyph.advance;

                    if (glyph.top >= 0) {
                        auto offset = y + prevGlyph.height + prevGlyph.top;
                        y1 = offset;
                        y2 = offset + glyph.height;
                    }
                    else {
                        auto offset = y + prevGlyph.top - glyph.height;
                        y1 = offset;
                        y2 = offset + glyph.height;
                    }

                    if (currChar == 0x0651 && isHarakat(prevChar)) {
                        y1 += prevGlyph.height;
                        y2 += prevGlyph.height;
                    }
                    else if (prevChar == 0x651) {
                        y1 -= prevGlyph.height;
                        y2 -= prevGlyph.height;
                    }
                }
            }

            // Add a quad for the current character
            float vertices[24] {
                x2 - italic * top,    y1, u2, v1,
                x1 - italic * top,    y1, u1, v1,
                x1 - italic * bottom, y2, u1, v2,
                x2 - italic * top,    y1, u2, v1,
                x1 - italic * bottom, y2, u1, v2,
                x2 - italic * bottom, y2, u2, v2
            };

            // Append vertices to the corresponding buffer
            auto& buffer = buffers[glyph.page];
            buffer.insert(buffer.end(), std::begin(vertices), std::end(vertices));
        }

        // Update the current bounds
        minX = std::min(minX, x + left - italic * bottom);
        maxX = std::max(maxX, x + right - italic * top);
        minY = std::min(minY, y + top);
        maxY = std::max(maxY, y + bottom);

        // Advance to the next character
        x += glyph.advance;

        prevChar = currChar;
    }

    // If we're using the underlined style, add the last line
    if (underlined) {
        float top = std::floor(y + underlineOffset - (underlineThickness / 2) + 0.5f);
        float bottom = top + std::floor(underlineThickness + 0.5f);

        float vertices[24] {
            x,   top,    1.f, 1.f,
            0.f, top,    1.f, 1.f,
            0.f, bottom, 1.f, 1.f,
            x,   top,    1.f, 1.f,
            0.f, bottom, 1.f, 1.f,
            x,   bottom, 1.f, 1.f
        };
        buffers[0].insert(buffers[0].end(), std::begin(vertices), std::end(vertices));
    }

    // If we're using the strike through style, add the last line across all characters
    if (strikeThrough) {
        float top = std::floor(y + strikeThroughOffset - (underlineThickness / 2) + 0.5f);
        float bottom = top + std::floor(underlineThickness + 0.5f);

        float vertices[24] {
            x,   top,    1.f, 1.f,
            0.f, top,    1.f, 1.f,
            0.f, bottom, 1.f, 1.f,
            x,   top,    1.f, 1.f,
            0.f, bottom, 1.f, 1.f,
            x,   bottom, 1.f, 1.f
        };
        buffers[0].insert(buffers[0].end(), std::begin(vertices), std::end(vertices));
    }

    mBoundsX = minX;
    mBoundsY = minY;
    mBoundsW = maxX - minX;
    mBoundsH = maxY - minY;

    mVertices.clear();
    mBufferIDs.clear();
    for (auto& it : buffers) {
        if (it.second.empty()) continue;

        mBufferIDs.push_back(it.first);

        mVertices[it.first].count = it.second.size() / 4u;
        mVertices[it.first].buffer.createVertex(
            it.second.size() * sizeof(float), it.second.data()
        );
    }
}
