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

#include "texture.hpp"

//==========================================================
// Holds information about a Font glyph
//==========================================================
struct Glyph
{
    float advance {0.f};
    float left    {0.f};
    float top     {0.f};
    float width   {0.f};
    float height  {0.f};
    uint32_t texLeft   {0};
    uint32_t texTop    {0};
    uint32_t texWidth  {0};
    uint32_t texHeight {0};
};

//==========================================================
// A base class for fonts
//==========================================================
class Font
{
public:
    Font() = default;
    virtual ~Font() = default;

    virtual const Glyph& glyph(uint32_t codePoint, uint32_t charSize, bool bold) const = 0;
    virtual float kerning(uint32_t first, uint32_t second, uint32_t charSize) const = 0;
    virtual float lineSpacing(uint32_t charSize) const = 0;
    virtual float underlinePosition(uint32_t charSize) const = 0;
    virtual float underlineThickness(uint32_t charSize) const = 0;
    virtual const Texture& texture(uint32_t charSize) const = 0;
};
