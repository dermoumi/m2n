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
#include "../config.hpp"
#include "../graphics/text.hpp"
#include "../graphics/rtltext.hpp"

//----------------------------------------------------------
// Declarations
//----------------------------------------------------------
using NxText = Text;
using NxFont = Font;
using NxArraybuffer = Arraybuffer;

//----------------------------------------------------------
NX_EXPORT NxText* nxTextNew()
{
    return new Text();
}

//----------------------------------------------------------
NX_EXPORT NxText* nxRtlTextNew()
{
    return new RtlText();
}

//----------------------------------------------------------
NX_EXPORT void nxTextRelease(NxText* text)
{
    delete text;
}

//----------------------------------------------------------
NX_EXPORT void nxTextSetString(NxText* text, const char* str)
{
    text->setString(str);
}

//----------------------------------------------------------
NX_EXPORT void nxTextSetU32String(NxText* text, const uint32_t* str)
{
    text->setString(reinterpret_cast<const char32_t*>(str));
}

//----------------------------------------------------------
NX_EXPORT void nxTextSetFont(NxText* text, const NxFont* font)
{
    text->setFont(*font);
}

//----------------------------------------------------------
NX_EXPORT void nxTextSetCharacterSize(NxText* text, uint32_t charSize)
{
    text->setCharacterSize(charSize);
}

//----------------------------------------------------------
NX_EXPORT void nxTextSetStyle(NxText* text, uint8_t style)
{
    text->setStyle(style);
}

//----------------------------------------------------------
NX_EXPORT void nxTextCharacterPosition(const Text* text, uint32_t index, float* posPtr)
{
    text->characterPosition(index, posPtr[0], posPtr[1]);
}

//----------------------------------------------------------
NX_EXPORT void nxTextBounds(const Text* text, float* boundsPtr)
{
    text->bounds(boundsPtr[0], boundsPtr[1], boundsPtr[2], boundsPtr[3]);
}

//----------------------------------------------------------
NX_EXPORT const NxArraybuffer* nxTextArraybuffer(const Text* text, uint32_t* vertexCount,
    uint32_t index)
{
    return text->arraybuffer(*vertexCount, index);
}

//----------------------------------------------------------
NX_EXPORT uint32_t* nxTextArraybufferIDs(const Text* text, uint32_t* count)
{
    return text->arraybufferIDs(count);
}
