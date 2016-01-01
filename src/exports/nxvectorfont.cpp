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

#include "../config.hpp"
#include "../graphics/vectorfont.hpp"

using NxFont = Font;
struct PHYSFS_File;

NX_EXPORT NxFont* nxVectorFontNew()
{
    return new VectorFont();
}

NX_EXPORT bool nxVectorFontOpenFromFile(NxFont* font, const char* filename)
{
    return static_cast<VectorFont*>(font)->open(filename);
}

NX_EXPORT bool nxVectorFontOpenFromMemory(NxFont* font, const void* buffer, size_t size)
{
    return static_cast<VectorFont*>(font)->open(buffer, size);
}

NX_EXPORT bool nxVectorFontOpenFromHandle(NxFont* font, PHYSFS_File* file)
{
    return static_cast<VectorFont*>(font)->open(file);
}

NX_EXPORT const char* nxVectorFontFamilyName(const NxFont* font)
{
    return static_cast<const VectorFont*>(font)->info().family.data();
}
