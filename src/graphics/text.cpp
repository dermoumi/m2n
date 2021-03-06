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

#include "text.hpp"
#include "renderdevice.hpp"
#include "../system/unicode.hpp"

#include <algorithm>
#include <cmath>

void Text::setString(const std::string& str)
{
    setString(Unicode::utf8To32(str));
}

void Text::setString(const std::u32string& str)
{
    mString = str;
    mNeedsUpdate = true;
}

void Text::setFont(const Font& font)
{
    if (mFont == &font) return;

    mFont = &font;
    mNeedsUpdate = true;
}

void Text::setCharacterSize(uint32_t charSize)
{
    if (mCharSize == charSize) return;

    mCharSize = charSize;
    mNeedsUpdate = true;
}

void Text::setStyle(uint8_t style)
{
    if (mStyle == style) return;

    mStyle = style;
    mNeedsUpdate = true;
}

const std::u32string& Text::string() const
{
    return mString;
}

const Font* Text::font() const
{
    return mFont;
}

uint32_t Text::characterSize() const
{
    return mCharSize;
}

uint8_t Text::style() const
{
    return mStyle;
}

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

void Text::bounds(float& x, float& y, float& w, float& h) const
{
    ensureGeometryUpdate();
    x = mBoundsX;
    y = mBoundsY;
    w = mBoundsW;
    h = mBoundsH;
}

VertexBuffer* Text::nextBuffer(uint32_t* index) const
{
    ensureGeometryUpdate();

    if (mNextPointer == mVertices.cend()) {
        mNextPointer = mVertices.cbegin();
        return nullptr;
    }

    *index = mNextPointer->first;
    return (mNextPointer++)->second.get();
}

void Text::ensureGeometryUpdate() const
{
    // mNextPointer = mVertices.cbegin();

    // If geometry is already up-to-date, do nothing
    if (!mNeedsUpdate) return;

    // Mark the geometry as updated
    mNeedsUpdate = false;

    // Reset variables
    mBoundsX = mBoundsY = mBoundsW = mBoundsH = 0;

    // No font or no string: nothing to draw
    if (!mFont || mString.empty()) {
        mNextPointer = mVertices.cbegin();
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
    float xBoundsY {xGlyph.top}, xBoundsH {xGlyph.height};

    float strikeThroughOffset = xBoundsY + xBoundsH / 2.f;

    // Precompute the variables needed by the algorithm
    float hspace = static_cast<float>(mFont->glyph(U' ', mCharSize, bold).advance);
    float vspace = static_cast<float>(mFont->lineSpacing(mCharSize));
    float x      = 0.f;
    float y      = static_cast<float>(mCharSize);

    // Create create two triangles for each character
    float minX = 0.f;
    float minY = static_cast<float>(mCharSize);
    float maxX = 0.f;
    float maxY = 0.f;
    uint32_t currChar = 0u, prevChar;

    for (size_t i = 0u; i < mString.size(); ++i) {
        prevChar = currChar;
        currChar = static_cast<uint32_t>(mString[i]);

        // Apply the kerning offset
        x += static_cast<float>(mFont->kerning(prevChar, currChar, mCharSize));

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

        float x1 = x + left;
        float x2 = x + right;
        float y1 = y + top;
        float y2 = y + bottom;

        if (glyph.texWidth != 0 && glyph.texHeight != 0) {
            float u1 = glyph.texLeft - 0.25f;
            float v1 = glyph.texTop - 0.25f;
            float u2 = glyph.texLeft + glyph.texWidth + 0.25f;
            float v2 = glyph.texTop + glyph.texHeight + 0.25f;

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
        minX = std::min(minX, x1 - italic * bottom);
        maxX = std::max(maxX, x2 - italic * top);
        minY = std::min(minY, y1);
        maxY = std::max(maxY, y2);

        // Advance to the next character
        x += glyph.advance;
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
    for (auto& it : buffers) {
        if (it.second.empty()) continue;

        auto buf = RenderDevice::instance().newVertexBuffer();
        buf->load(
            it.second.data(),
            static_cast<uint32_t>(it.second.size() * sizeof(float)),
            4 * sizeof(float)
        );

        mVertices[it.first] = std::shared_ptr<VertexBuffer>(buf);
    }

    mNextPointer = mVertices.cbegin();
}
