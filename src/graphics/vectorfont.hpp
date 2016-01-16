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

#pragma once
#include "../config.hpp"
#include "font.hpp"

#include <string>
#include <memory>
#include <vector>
#include <map>

struct PHYSFS_File;

// Represents a vector font
class VectorFont : public Font
{
public:
    struct Info
    {
        std::string family;
    };

public:
    VectorFont() = default;
    virtual ~VectorFont();

    bool open(const std::string& filename);
    bool open(const void* data, size_t size);
    bool open(PHYSFS_File* file);

    const Info& info() const;

    virtual const Glyph& glyph(uint32_t codePoint, uint32_t charSize, bool bold) const;
    virtual float kerning(uint32_t first, uint32_t second, uint32_t charSize) const;
    virtual float lineSpacing(uint32_t charSize) const;
    virtual float underlinePosition(uint32_t charSize) const;
    virtual float underlineThickness(uint32_t charSize) const;
    virtual const Texture* texture(uint32_t charSize, uint32_t index) const;

private:
    using GlyphTable = std::map<uint32_t, Glyph>;
    struct Row
    {
        Row(uint32_t top, uint32_t height);

        uint32_t top {0u};
        uint32_t width {0u};
        uint32_t height {0u};
    };

    struct Page
    {
        Page();
        Page(Page&& other);

        GlyphTable               glyphs;
        std::shared_ptr<Texture> texture;
        uint32_t                 nextRow {3u};
        std::vector<Row>         rows;
    };

private:
    void cleanup();
    Glyph loadGlyph(uint32_t codePoint, uint32_t charSize, bool bold) const;
    bool findGlyphRect(Page* page, uint32_t width, uint32_t height, uint32_t& coordsL,
        uint32_t& coordsT, uint32_t& coordsR, uint32_t& coordsB) const;
    bool ensureSize(uint32_t charSize) const;

private:
    class FreetypeHandle;
    class FileWrapper;
    using FreetypePtr = std::shared_ptr<FreetypeHandle>;
    using PageTable   = std::map<uint32_t, std::vector<Page>>;
    using FilePtr     = std::shared_ptr<FileWrapper>;

    Info        mInfo;
    FreetypePtr mFreetype;
    FilePtr     mFile; // Used by open(const std::string&);
    mutable PageTable            mPages;
    mutable std::vector<uint8_t> mPixelBuffer;
};
