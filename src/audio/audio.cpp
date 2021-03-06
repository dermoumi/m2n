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

#include "audio.hpp"

#include <physfs/physfs.h>

SoLoud::Soloud& Audio::instance()
{
    static SoLoud::Soloud soloud;
    return soloud;
}

Audio::File::~File()
{
    if (mFile) PHYSFS_close(mFile);
}

bool Audio::File::open(const char* filename)
{
    return (mFile = PHYSFS_openRead(filename)) != nullptr;
}

int Audio::File::eof()
{
    return PHYSFS_eof(mFile);
}

unsigned int Audio::File::read(unsigned char* dst, unsigned int bytes)
{
    if (!mFile) return 0u;
    auto status = PHYSFS_readBytes(mFile, dst, bytes);
    if (status < 0) return 0u;
    return static_cast<unsigned int>(status);
}

unsigned int Audio::File::length()
{
    if (!mFile) return 0u;
    auto status = PHYSFS_fileLength(mFile);
    if (status < 0) return 0u;
    return static_cast<unsigned int>(status);
}

void Audio::File::seek(int offset)
{
    if (!mFile) return;
    PHYSFS_seek(mFile, offset);
}

unsigned int Audio::File::pos()
{
    if (!mFile) return 0u;
    auto status = PHYSFS_tell(mFile);
    if (status < 0) return 0u;
    return static_cast<unsigned int>(status);
}
