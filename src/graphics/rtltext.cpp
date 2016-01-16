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

#include "rtltext.hpp"
#include "renderdevice.hpp"

#include <algorithm>
#include <cmath>
#include <set>

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

void RtlText::ensureGeometryUpdate() const
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
    float italic             = (mStyle & Italic) ? -0.104f : 0.f; // 6 degrees
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

    float lastExtraSpace = 0.f;

    for (size_t i = 0u; i < mString.size(); ++i) {
        prevChar = currChar;
        currChar = static_cast<uint32_t>(mString[i]);

        // Apply the kerning offset
        x += static_cast<float>(mFont->kerning(currChar, prevChar, mCharSize));

        // If we're using the underlined style and there's a new line, draw a line
        if (underlined && (currChar == U'\n')) {
            float top = std::floor(y + underlineOffset - (underlineThickness / 2) + 0.5f);
            float bottom = top + std::floor(underlineThickness + 0.5f);

            float vertices[24] {
                0.f,                top,    1.f, 1.f,
                lastExtraSpace - x, top,    1.f, 1.f,
                lastExtraSpace - x, bottom, 1.f, 1.f,
                0.f,                top,    1.f, 1.f,
                lastExtraSpace - x, bottom, 1.f, 1.f,
                0.f,                bottom, 1.f, 1.f
            };
            buffers[0].insert(buffers[0].end(), std::begin(vertices), std::end(vertices));
        }

        // If we're using strike through style and there's a new line, draw a line accross
        // all characters
        if (strikeThrough && (currChar == U'\n')) {
            float top = std::floor(y + strikeThroughOffset - (underlineThickness / 2) + 0.5f);
            float bottom = top + std::floor(underlineThickness + 0.5f);

            float vertices[24] {
                0.f, top,           1.f, 1.f,
                lastExtraSpace - x, top,    1.f, 1.f,
                lastExtraSpace - x, bottom, 1.f, 1.f,
                0.f,                top,    1.f, 1.f,
                lastExtraSpace - x, bottom, 1.f, 1.f,
                0.f,                bottom, 1.f, 1.f
            };
            buffers[0].insert(buffers[0].end(), std::begin(vertices), std::end(vertices));
        }

        // Handle special characters
        if (currChar == U' ' || currChar == U'\t' || currChar == U'\n') {
            // Update the current bounds (min coodinates)
            minX = std::min(minX, -x);
            minY = std::min(minY, y);

            switch (currChar) {
                case U' ':  x += hspace;        break;
                case U'\t': x += hspace * 4;    break;
                case U'\n': y += vspace; x = 0; break;
            }

            // Update the current bounds (max coordinates)
            maxX = std::max(maxX, -x);
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

        x += glyph.advance;

        float x1 = -x + left;
        float x2 = -x + right;
        float y1 = y + top;
        float y2 = y + bottom;

        lastExtraSpace = glyph.advance - right;

        if (glyph.texWidth != 0 && glyph.texHeight != 0) {
            float u1 = glyph.texLeft - 0.25f;
            float v1 = glyph.texTop - 0.25f;
            float u2 = glyph.texLeft + glyph.texWidth + 0.25f;
            float v2 = glyph.texTop + glyph.texHeight + 0.25f;

            if (prevChar && isHarakat(currChar)) {
                x -= glyph.advance;

                const auto& prevGlyph = mFont->glyph(prevChar, mCharSize, bold);
                auto xOffset = (prevGlyph.advance + glyph.width) / 2.f;
                x1 += xOffset;
                x2 += xOffset;

                if (glyph.top >= 0) {
                    auto offset = y + prevGlyph.height + prevGlyph.top + mCharSize / 20;
                    y1 = offset;
                    y2 = offset + glyph.height;
                }
                else {
                    auto offset = y + prevGlyph.top - glyph.height - mCharSize / 20;
                    y1 = offset;
                    y2 = offset + glyph.height;
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
        minX = std::min(minX, x1 - italic * bottom);
        maxX = std::max(maxX, x2 - italic * top);
        minY = std::min(minY, y1);
        maxY = std::max(maxY, y2);
    }

    // If we're using the underlined style, add the last line
    if (underlined) {
        float top = std::floor(y + underlineOffset - (underlineThickness / 2) + 0.5f);
        float bottom = top + std::floor(underlineThickness + 0.5f);

        float vertices[24] {
            0.f,                top,    1.f, 1.f,
            lastExtraSpace - x, top,    1.f, 1.f,
            lastExtraSpace - x, bottom, 1.f, 1.f,
            0.f,                top,    1.f, 1.f,
            lastExtraSpace - x, bottom, 1.f, 1.f,
            0.f,                bottom, 1.f, 1.f
        };
        buffers[0].insert(buffers[0].end(), std::begin(vertices), std::end(vertices));
    }

    // If we're using the strike through style, add the last line across all characters
    if (strikeThrough) {
        float top = std::floor(y + strikeThroughOffset - (underlineThickness / 2) + 0.5f);
        float bottom = top + std::floor(underlineThickness + 0.5f);

        float vertices[24] {
            0.f,                top,    1.f, 1.f,
            lastExtraSpace - x, top,    1.f, 1.f,
            lastExtraSpace - x, bottom, 1.f, 1.f,
            0.f,                top,    1.f, 1.f,
            lastExtraSpace - x, bottom, 1.f, 1.f,
            0.f,                bottom, 1.f, 1.f
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

        mVertices[it.first] = std::shared_ptr<VertexBuffer>(
            RenderDevice::instance().newVertexBuffer()
        );
        mVertices[it.first]->load(
            it.second.data(),
            static_cast<uint32_t>(it.second.size() * sizeof(float)),
            4 * sizeof(float)
        );
    }
}
