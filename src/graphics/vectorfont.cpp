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

#include "image.hpp"
#include "renderdevice.hpp"
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
        if (count == 0) return 0u;
        
        auto status = PHYSFS_readBytes(file, buffer, count);

        // If succeeded, return status as the number of bytes read
        if (status > 0u) return static_cast<unsigned long>(status);
    }
    
    // "This function might be called to perform a seek or skip operation with a ‘count’ of 0.
    // "A non-zero return value then indicates an error.
    return count == 0u ? 1u : 0u;
}

static void close(FT_Stream)
{
    // Nothing to do
}

//----------------------------------------------------------
class VectorFont::FileWrapper
{
public:
    FileWrapper() : handle() {}
    ~FileWrapper() {if (handle) PHYSFS_close(handle);}
    PHYSFS_File* handle;
};

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
    // Cleanup thee previous resources
    mFile = std::make_shared<FileWrapper>();

    mFile->handle = PHYSFS_openRead(filename.data());
    if (!mFile->handle) {
        Log::error("Unable to load font \"" + filename);
        return false;
    }

    return open(mFile->handle);
}

//----------------------------------------------------------
bool VectorFont::open(const void* data, size_t size)
{
    // Cleanup the previous resources
    cleanup();
    mFreetype = std::make_shared<FreetypeHandle>();

    // Initialize Freetype
    FT_Library library;
    if (FT_Init_FreeType(&library) != 0) {
        Log::error("Failed to load font from memory: Failed to initialize Freetype");
        return false;
    }
    mFreetype->library = library;

    // Load the new font face from the buffer
    auto ftData = static_cast<const FT_Byte*>(data);
    auto ftSize = static_cast<FT_Long>(size);

    FT_Face face;
    if (FT_New_Memory_Face(mFreetype->library, ftData, ftSize, 0, &face) != 0) {
        Log::error("Failed to load font from memory: Failed to create the font face");
        return false;
    }

    // Select the unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0) {
        Log::error("Failed to load font from memory: Failed to set the Unicode character set");
        FT_Done_Face(face);
        return false;
    }

    // Stored the loaded font
    mFreetype->face = face;

    // Store the font information
    mInfo.family = face->family_name ? face->family_name : "";

    return true;
}

//----------------------------------------------------------
bool VectorFont::open(PHYSFS_File* file)
{
    // Cleanup the previous resources
    cleanup();
    mFreetype = std::make_shared<FreetypeHandle>();

    // Initialize Freetype
    FT_Library library;
    if (FT_Init_FreeType(&library) != 0) {
        Log::error("Failed to load font from file: Failed to initialize Freetype");
        return false;
    }
    mFreetype->library = library;

    // Make sure that the stream's reading position is at the beginning
    if (!PHYSFS_seek(file, 0)) {
        Log::error("Failed to load font from file: cannot seek beginning of the file");
        return false;
    }

    // Setup Freetype's callbacks wrapper
    FT_StreamRec* rec = new FT_StreamRec();
    std::memset(rec, 0, sizeof(FT_StreamRec));
    rec->base = nullptr;
    rec->size = static_cast<unsigned long>(PHYSFS_fileLength(file));
    rec->pos  = 0;
    rec->descriptor.pointer = file;
    rec->read = &read;
    rec->close = &close;

    // Setup the Freetype callbacks that will read our file
    FT_Open_Args args;
    args.flags  = FT_OPEN_STREAM;
    args.stream = rec;
    args.driver = 0;

    // Load the new font face from the specified stream
    FT_Face face;
    if (FT_Open_Face(mFreetype->library, &args, 0, &face) != 0) {
        Log::error("Failed to load font from file: failde to create the font face");
        delete rec;
        return false;
    }

    // Select the Unicode character map
    if (FT_Select_Charmap(face, FT_ENCODING_UNICODE) != 0) {
        Log::error("Failed to load font from file: failed to set the Unicode character set");
        FT_Done_Face(face);
        delete rec;
        return false;
    }

    // Store our font
    mFreetype->face = face;
    mFreetype->streamRec = rec;

    // Store the font information
    mInfo.family = face->family_name ? face->family_name : "";

    return true;
}

//----------------------------------------------------------
const VectorFont::Info& VectorFont::info() const
{
    return mInfo;
}

//----------------------------------------------------------
const Glyph& VectorFont::glyph(uint32_t codePoint, uint32_t charSize, bool bold) const
{
    // Build the key by combining the codepoint and the bold flag
    uint32_t key = ((bold ? 1 : 0) << 31) | codePoint;

    // Cycle through pages
    for (auto& page : mPages[charSize]) {
        // Get the page corresponding to the character size
        auto& glyphs = page.glyphs;

        // Search the glyph from the cache
        auto it = glyphs.find(key);
        if (it != glyphs.end()) {
            // Found: just return it
            return it->second;
        }
    }

    // Not found: we have to load it
    Glyph glyph = loadGlyph(codePoint, charSize, bold);
    if (mPages[charSize].empty()) {
        static Glyph invalid;
        invalid = glyph;
        invalid.page = 0;
        return invalid;
    }
    return mPages[charSize][glyph.page].glyphs.emplace(key, glyph).first->second;
}

//----------------------------------------------------------
float VectorFont::kerning(uint32_t first, uint32_t second, uint32_t charSize) const
{
    // Special case where first or second character is 0 (NUL character)
    if (first == 0 || second == 0) return 0.f;

    auto face = mFreetype ? mFreetype->face : nullptr;
    if (face && FT_HAS_KERNING(face) && ensureSize(charSize)) {
        // Convert the characters to indices
        FT_UInt index1 = FT_Get_Char_Index(face, first);
        FT_UInt index2 = FT_Get_Char_Index(face, second);

        // Get the kerning vector
        FT_Vector kerning;
        FT_Get_Kerning(face, index1, index2, FT_KERNING_DEFAULT, &kerning);

        // X advance is already in pixels for bitmap fonts
        if (!FT_IS_SCALABLE(face)) return static_cast<float>(kerning.x);

        // Return the X advance
        return kerning.x / static_cast<float>(1 << 6);
    }

    // Invalid font or no kerning
    return 0.f;
}

//----------------------------------------------------------
float VectorFont::lineSpacing(uint32_t charSize) const
{
    auto face = mFreetype ? mFreetype->face : nullptr;

    if (face && ensureSize(charSize)) {
        return face->size->metrics.height / static_cast<float>(1 << 6);
    }

    return 0.f;
}

//----------------------------------------------------------
float VectorFont::underlinePosition(uint32_t charSize) const
{
    FT_Face face = mFreetype ? mFreetype->face : nullptr;

    if (face && ensureSize(charSize)) {
        // Return a fixed position if the font is a bitmap font
        if (!FT_IS_SCALABLE(face)) return charSize / 10.f;

        auto mulFix = FT_MulFix(face->underline_position, face->size->metrics.y_scale);
        return -mulFix / static_cast<float>(1 << 6);
    }

    return 0.f;
}

//----------------------------------------------------------
float VectorFont::underlineThickness(uint32_t charSize) const
{
    FT_Face face = mFreetype ? mFreetype->face : nullptr;

    if (face && ensureSize(charSize)) {
        // Return a fixed thickness if font is a bitmap font
        if (!FT_IS_SCALABLE(face)) return charSize / 14.f;

        auto mulFix = FT_MulFix(face->underline_thickness, face->size->metrics.y_scale);
        return mulFix / static_cast<float>(1 << 6);
    }

    return 0.f;
}

//----------------------------------------------------------
const Texture* VectorFont::texture(uint32_t charSize, uint32_t index) const
{
    return &mPages[charSize][index].texture;
}

//----------------------------------------------------------
void VectorFont::cleanup()
{
    mFreetype = nullptr;
    mPages.clear();
    mPixelBuffer.clear();
}

//----------------------------------------------------------
Glyph VectorFont::loadGlyph(uint32_t codePoint, uint32_t charSize, bool bold) const
{
    // The glyph to return
    Glyph glyph;

    // Shortcut to our glyph
    auto face = mFreetype ? mFreetype->face : nullptr;
    if (!face) return glyph;

    // Set the character size
    if (!ensureSize(charSize)) return glyph;

    // Make sure the character exists
    auto glyphIndex = FT_Get_Char_Index(face, codePoint);
    if (glyphIndex == 0) return glyph;

    // Load the glyph corresponding to the code point
    if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_TARGET_NORMAL | FT_LOAD_FORCE_AUTOHINT) != 0) {
        return glyph;
    }

    // Retrieve the glyph
    FT_Glyph glyphDesc;
    if (FT_Get_Glyph(face->glyph, &glyphDesc) != 0) return glyph;

    // Apply bold if necessary -- first technique: using outline (highest quality)
    FT_Pos weight = 1 << 6;
    bool outline = (glyphDesc->format == FT_GLYPH_FORMAT_OUTLINE);
    if (bold && outline) {
        FT_OutlineGlyph outlineGlyph = (FT_OutlineGlyph) glyphDesc;
        FT_Outline_Embolden(&outlineGlyph->outline, weight);
    }

    // Convert glyph to a bitmap (rasterize it)
    FT_Glyph_To_Bitmap(&glyphDesc, FT_RENDER_MODE_NORMAL, 0, 1);
    FT_Bitmap& bitmap = reinterpret_cast<FT_BitmapGlyph>(glyphDesc)->bitmap;

    // Apply bold if necessary --fallback technique using bitmap (lower quality)
    if (bold && !outline) FT_Bitmap_Embolden(mFreetype->library, &bitmap, weight, weight);

    // Compute the glyph's advance offset
    glyph.advance = face->glyph->metrics.horiAdvance / static_cast<float>(1 << 6);
    if (bold) glyph.advance += weight / static_cast<float>(1 << 6);

    int width  = bitmap.width;
    int height = bitmap.rows;

    if (width > 0 && height > 0) {
        constexpr auto padding = 1u;

        // Get the glyphs page corresponding to the charcater size
        Page* page = nullptr;
        for (uint32_t i = 0; !page; ++i) {
            if (i < mPages[charSize].size()) {
                page = &mPages[charSize][i];

                bool found = findGlyphRect(
                    page, width + 2 * padding, height + 2 * padding,
                    glyph.texLeft, glyph.texTop, glyph.texWidth, glyph.texHeight
                );

                if (found) {
                    glyph.page = i;
                }
                else {
                    page = nullptr;
                }
            }
            else {
                mPages[charSize].emplace_back();
                page = &mPages[charSize][i];

                // Find a good position for the new glyph into the texture
                bool found = findGlyphRect(
                    page, width + 2 * padding, height + 2 * padding,
                    glyph.texLeft, glyph.texTop, glyph.texWidth, glyph.texHeight
                );

                if (found) {
                    glyph.page = i;
                }
                else {
                    Log::error(
                        "Failed to add a new character to the font: "
                        "The maximum texture size has been reached"
                    );

                    glyph.texLeft = 0;
                    glyph.texTop = 0;
                    glyph.texWidth = 2;
                    glyph.texHeight = 2;
                    glyph.page = 0;
                }
            }
        }

        // Make sure the texture data is poositioned in the center of the allocated texture rect
        glyph.texLeft += padding;
        glyph.texTop  += padding;
        glyph.texWidth  -= 2 * padding;
        glyph.texHeight -= 2 * padding;

        // Compute the glyph's bounding box
        glyph.left   =  face->glyph->metrics.horiBearingX / static_cast<float>(1 << 6);
        glyph.top    = -face->glyph->metrics.horiBearingY / static_cast<float>(1 << 6);
        glyph.width  =  face->glyph->metrics.width  / static_cast<float>(1 << 6);
        glyph.height =  face->glyph->metrics.height / static_cast<float>(1 << 6);

        glyph.valid = true;

        if (glyph.texWidth > 0 && glyph.texHeight > 0) {
            // Extract the glyph's pixels from the bitmap
            mPixelBuffer.resize(width * height * 4, 255);
            const uint8_t* pixels = bitmap.buffer;
            if (bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
                // Pixels are 1 bit monochrome values
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        // The color channels remain white, just fill the alpha channel
                        size_t index = (x + y * width) * 4 + 3;
                        mPixelBuffer[index] = ((pixels[x / 8]) & (1 << (7 - (x % 8)))) ? 255 : 0;
                    }

                    pixels += bitmap.pitch;
                }
            }
            else {
                // Pixels are 8 bits gray levels
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        // The color channls remain white, just fill the alpha channel
                        size_t index = (x + y * width) * 4 + 3;
                        mPixelBuffer[index] = pixels[x];
                    }

                    pixels += bitmap.pitch;
                }
            }

            page->texture.setData(
                &mPixelBuffer[0], glyph.texLeft, glyph.texTop, 0, glyph.texWidth, glyph.texHeight,
                1, 0, 0
            );
        }
    }

    // Delete the FT_Glyph
    FT_Done_Glyph(glyphDesc);

    // TODO: Force an OpenGL flush
    RenderDevice::instance().sync();

    // Done
    return glyph;
}

//----------------------------------------------------------
bool VectorFont::findGlyphRect(Page* page, uint32_t width, uint32_t height, uint32_t& coordsX,
    uint32_t& coordsY, uint32_t& coordsW, uint32_t& coordsH) const
{
    // Find the line that fits well the glyph
    Row* currentRow = nullptr;
    for (auto& row : page->rows) {
        float ratio = static_cast<float>(height) / row.height;

        // Ignore rows that are either too small or too high
        if (ratio < 0.7f || ratio > 1.f) continue;

        // Check if there's enough horizontal space left in the row
        if (width > page->texture.texWidth() - row.width) continue;

        // Make sure that the current row passed all the tests: we can select it
        currentRow = &row;
        break;
    }

    if (!currentRow) {
        uint32_t rowHeight = height * 1.1;
        while (
            page->nextRow + rowHeight >= page->texture.texHeight()
            || width >= page->texture.texWidth()
        ) {
            // Not enough space: resize the texture if possible
            uint16_t texWidth  = page->texture.texWidth();
            uint16_t texHeight = page->texture.texHeight();
            uint16_t maxSize   = std::min<uint16_t>(4096u, Texture::maxSize());

            if (texWidth * 2 > maxSize || texHeight * 2 > maxSize) {
                // Reached max size, try next page
                return false;
            }

            // Make the texture twice as big
            uint32_t bufSize = texWidth * texHeight * 16;
            uint8_t* buffer = new uint8_t[bufSize];
            page->texture.data(buffer, 0, 0);

            Image image;
            image.create(texWidth * 2, texHeight * 2, 255, 255, 255, 0);
            image.copy(buffer, 0, 0, texWidth, 0, 0, texWidth, texHeight, false);

            // page->texture = Texture();
            auto ok = page->texture.create(0, 1, texWidth * 2, texHeight * 2, 1, true, true, false);
            if (ok == 0) {
                page->texture.setData(image.getPixelsPtr(), -1, -1, -1, -1, -1, -1, 0, 0);
            }
            else {
                Log::error("Could not create a new font texture.");
                return false;
            }
        }

        // We can now create the new row
        page->rows.push_back({page->nextRow, rowHeight});
        page->nextRow += rowHeight;
        currentRow = &page->rows.back();
    }

    coordsX = currentRow->width;
    coordsY = currentRow->top;
    coordsW = width;
    coordsH = height;

    currentRow->width += width;

    return true;
}

//----------------------------------------------------------
bool VectorFont::ensureSize(uint32_t charSize) const
{
    // FT_Set_Pixel_Sizes is an expensive function, so we must call it only when necessary
    auto face = mFreetype->face;
    FT_UShort currentSize = face->size->metrics.x_ppem;

    if (currentSize == charSize) return true;

    FT_Error result = FT_Set_Pixel_Sizes(face, 0, charSize);

    // In he case of bitmap fonts, resizing can fail if the requested size is unavailable
    if (result == FT_Err_Invalid_Pixel_Size && !FT_IS_SCALABLE(face)) {
        std::stringstream availableSizes;
        for (int i = 0; i < face->num_fixed_sizes; ++i) {
            availableSizes << face->available_sizes[i].height << " ";
        }

        Log::error("Failed to set bitmap font size to %d", charSize);
        Log::error("Available sizes are: " + availableSizes.str());
    }

    return result == FT_Err_Ok;
}

//----------------------------------------------------------
VectorFont::Row::Row(uint32_t rowTop, uint32_t rowHeight) :
    top(rowTop),
    height(rowHeight)
{
    // Nothing else to do
}

//----------------------------------------------------------
VectorFont::Page::Page()
{
    // Make sure that the texture is initialize by default
    Image image;
    image.create(128, 128, 255, 255, 255, 0);

    // Reserve a 2x2 white space for texture underlines
    for (int x = 0; x < 2; ++x) {
        for (int y = 0; y < 2; ++y) {
            image.setPixel(x, y, 255, 255, 255, 255);
        }
    }

    // Create texture
    texture.create(0, 1, 128, 128, 1, true, true, false);
    texture.setData(image.getPixelsPtr(), -1, -1, -1, -1, -1, -1, 0, 0);
}

//--------------------------------------------------------
VectorFont::Page::Page(Page&& other)
{
    std::swap(glyphs, other.glyphs);
    std::swap(texture, other.texture);
    std::swap(rows, other.rows);
    nextRow = other.nextRow;
}
