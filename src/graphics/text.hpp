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

#include "font.hpp"
#include "arraybuffer.hpp"
#include <string>
#include <vector>
#include <map>

//----------------------------------------------------------
class Text
{
public:
    enum Style : uint8_t
    {
        Regular       = 0,
        Bold          = 1 << 0,
        Italic        = 1 << 1,
        Underlined    = 1 << 2,
        StrikeThrough = 1 << 3
    };

public:
    Text() = default;

    void setString(const std::string& str);
    void setString(const std::u32string& str);
    void setFont(const Font& font);
    void setCharacterSize(uint32_t charSize);
    void setStyle(uint8_t style);

    const std::u32string& string() const;
    const std::string& utf8String() const;
    const Font* font() const;
    uint32_t characterSize() const;
    uint8_t style() const;

    void characterPosition(size_t index, float& x, float& y) const;
    void bounds(float& x, float& y, float& w, float& h) const;

    const Arraybuffer* arraybuffer(uint32_t& vertexCount, uint32_t index) const;
    uint32_t* arraybufferIDs(uint32_t* count) const;

private:
    void ensureGeometryUpdate() const;

    struct VertexList
    {
        Arraybuffer buffer;
        uint32_t count {0u};
    };

    std::u32string mString;
    const Font*    mFont {nullptr};
    uint32_t       mCharSize {30u};
    uint8_t        mStyle {Regular};
    mutable std::map<uint32_t, VertexList> mVertices;
    mutable std::vector<uint32_t> mBufferIDs;
    mutable float mBoundsX {0.f}, mBoundsY {0.f}, mBoundsW {0.f}, mBoundsH {0.f};
    mutable bool mNeedsUpdate {false};
};
