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
#include "vectorfont.hpp"

#include "../system/log.hpp"
#include <physfs/physfs.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include <cstdlib>
#include <cstring>
#include <sstream>

//----------------------------------------------------------
static unsigned long read(FT_Stream rec, unsigned long offset, unsigned char* buffer,
    unsigned long count)
{
    auto file = static_cast<PHYSFS_File*>(rec->descriptor.pointer);
    if (PHYSFS_seek(file, offset)) {
        if (count > 0u) {
            auto status = PHYSFS_readBytes(file, buffer, count);
            if (status > 0u) {
                return static_cast<unsigned long>(status);
            }
            else {
                return 0u;
            }
        }
        else {
            return 0u;
        }
    }
    else {
        return count > 0u ? 0u : 1u;
    }
}

static void close(FT_Stream)
{
    // Nothing to do
}

constexpr uint32_t glyphPadding = 1u;

//----------------------------------------------------------
class VectorFont::FreetypeHandle
{
public:
    ~FreetypeHandle();

    FT_Library    library   {nullptr};
    FT_StreamRec* streamRec {nullptr};
    FT_Face       face      {nullptr};
};

//----------------------------------------------------------
VectorFont::FreetypeHandle::~FreetypeHandle()
{
    if (face) FT_Done_Face(face);

    delete streamRec;

    if (library) FT_Done_FreeType(library);
}

//----------------------------------------------------------
VectorFont::~VectorFont()
{
    cleanup();
}

//----------------------------------------------------------
bool VectorFont::open(const std::string& filename)
{
    // TODO
    return false;
}

//----------------------------------------------------------
bool VectorFont::open(const void* data, size_t size)
{
    // TODO
    return false;
}

//----------------------------------------------------------
bool VectorFont::open(PHYSFS_File* file, bool closeFile)
{
    // TODO
    return false;
}

//----------------------------------------------------------
const VectorFont::Info& VectorFont::getInfo() const
{
    return mInfo;
}

//----------------------------------------------------------
const Glyph& VectorFont::glyph(uint32_t codePoint, uint32_t charSize, bool bold) const
{
    // TODO
    static Glyph invalidGlyph;
    return invalidGlyph;
}

//----------------------------------------------------------
float VectorFont::kerning(uint32_t first, uint32_t second, uint32_t charSize) const
{
    // TODO
    return 0.f;
}

//----------------------------------------------------------
float VectorFont::lineSpacing(uint32_t charSize) const
{
    // TODO
    return 0.f;
}

//----------------------------------------------------------
float VectorFont::underlinePosition(uint32_t charSize) const
{
    // TODO
    return 0.f;
}

//----------------------------------------------------------
float VectorFont::underlineThickness(uint32_t charSize) const
{
    // TODO
    return 0.f;
}

//----------------------------------------------------------
const Texture& VectorFont::texture(uint32_t charSize) const
{
    // TODO
    static Texture invalidTexture;
    return invalidTexture;
}

//----------------------------------------------------------
void VectorFont::cleanup()
{
    // TODO
}

//----------------------------------------------------------
Glyph VectorFont::loadGlyph(uint32_t codePoint, uint32_t charSize, bool bold) const
{
    // TODO
    return {};
}

//----------------------------------------------------------
bool VectorFont::findGlyphRect(Page& page, uint32_t width, uint32_t height, uint32_t& coordsL,
    uint32_t& coordsT, uint32_t& coordsR, uint32_t& coordsB) const
{
    // TODO
    return false;
}

//----------------------------------------------------------
bool VectorFont::ensureSize(uint32_t charSize) const
{
    // TODO
}