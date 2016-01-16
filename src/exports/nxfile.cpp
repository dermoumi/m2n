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
#include "../system/filesystem.hpp"

#include <physfs/physfs.h>
#include <sstream>
#include <cstring>

// Guarentees correct values regardless of precision difference.
//  eg: no more 2.6f --> 2.59654d
static float d2f(double val)
{
    float ret;
    std::stringstream ss;
    ss << val;
    ss >> ret;
    return ret;
}

static double f2d(float val)
{
    double ret;
    std::stringstream ss;
    ss << val;
    ss >> ret;
    return ret;
}

NX_EXPORT const char* nxFsGetError()
{
    thread_local std::string message;

    message = Filesystem::getErrorMessage();
    return message.data();
}

NX_EXPORT PHYSFS_File* nxFsOpenRead(const char* fileName)
{
    return PHYSFS_openRead(fileName);
}

NX_EXPORT PHYSFS_File* nxFsOpenWrite(const char* fileName)
{
    return PHYSFS_openWrite(fileName);
}

NX_EXPORT bool nxFsFlush(PHYSFS_File* handle)
{
    return PHYSFS_flush(handle) != 0;
}

NX_EXPORT void nxFsClose(PHYSFS_File* handle)
{
    if (handle) PHYSFS_close(handle);
}

NX_EXPORT bool nxFsSize(PHYSFS_File* handle, size_t* size)
{
    auto status = PHYSFS_fileLength(handle);
    if (status < 0) return false;

    *size = static_cast<size_t>(status);
    return true;
}

NX_EXPORT bool nxFsTell(PHYSFS_File* handle, size_t* position)
{
    auto status = PHYSFS_tell(handle);
    if (status < 0) return false;

    *position = static_cast<size_t>(status);
    return true;
}

NX_EXPORT bool nxFsSeek(PHYSFS_File* handle, size_t position)
{
    return PHYSFS_seek(handle, position) != 0;
}

NX_EXPORT bool nxFsRead(PHYSFS_File* handle, void* buffer, size_t length, size_t* readBytes)
{
    auto status = PHYSFS_readBytes(handle, buffer, length);
    if (status < 0) return false;

    if (readBytes) *readBytes = static_cast<size_t>(status);

    return true;
}

NX_EXPORT bool nxFsReadS8(PHYSFS_File* handle, int8_t* val)
{
    return nxFsRead(handle, val, 1, nullptr);
}

NX_EXPORT bool nxFsReadS16(PHYSFS_File* handle, int16_t* val)
{
    return PHYSFS_readSLE16(handle, val) != 0;
}

NX_EXPORT bool nxFsReadS32(PHYSFS_File* handle, int32_t* val)
{
    return PHYSFS_readSLE32(handle, val) != 0;
}

NX_EXPORT bool nxFsReadS64(PHYSFS_File* handle, int64_t* val)
{
    static_assert(sizeof(int64_t) == sizeof(PHYSFS_sint64), "the size of int64_t is invalid");

    return PHYSFS_readSLE64(handle, reinterpret_cast<PHYSFS_sint64*>(val)) != 0;
}

NX_EXPORT bool nxFsReadU8(PHYSFS_File* handle, uint8_t* val)
{
    return nxFsRead(handle, reinterpret_cast<char*>(val), 1, nullptr) != 0;
}

NX_EXPORT bool nxFsReadU16(PHYSFS_File* handle, uint16_t* val)
{
    return PHYSFS_readULE16(handle, val) != 0;
}

NX_EXPORT bool nxFsReadU32(PHYSFS_File* handle, uint32_t* val)
{
    return PHYSFS_readULE32(handle, val) != 0;
}

NX_EXPORT bool nxFsReadU64(PHYSFS_File* handle, uint64_t* val)
{
    static_assert(sizeof(uint64_t) == sizeof(PHYSFS_uint64), "the size of int64_t is invalid");

    return PHYSFS_readULE64(handle, reinterpret_cast<PHYSFS_uint64*>(val)) != 0;
}

NX_EXPORT bool nxFsReadFloat(PHYSFS_File* handle, double* val)
{
    static_assert(sizeof(float) == 4, "Float is not 4-bytes long");

    // while reinterpret_cast usually works, it can have undefined behavior.
    // we don't want that
    union
    {
        uint32_t u;
        float f;
    } u;

    if (!nxFsReadU32(handle, &u.u)) return false;

    *val = f2d(u.f);
    return true;
}

NX_EXPORT bool nxFsReadDouble(PHYSFS_File* handle, double* val)
{
    static_assert(sizeof(double) == 8, "Double is not 8-bytes long");

    // while reinterpret_cast usually works, it can have undefined behavior.
    // we don't want that
    union
    {
        uint64_t u;
        double d;
    } u;

    if (!nxFsReadU64(handle, &u.u)) return false;

    *val = u.d;
    return true;
}

NX_EXPORT const char* nxFsReadString(PHYSFS_File* handle)
{
    thread_local std::string str;
    char c;

    do {
        if (!nxFsRead(handle, &c, 1u, nullptr)) return nullptr;

        str += c;
    }
    while (c != '\0');

    return str.data();
}

NX_EXPORT bool nxFsWrite(PHYSFS_File* handle, const void* buffer, size_t length,
    size_t* writtenBytes)
{
    auto status = PHYSFS_writeBytes(handle, buffer, length);
    if (status < 0) return false;

    if (writtenBytes) *writtenBytes = static_cast<size_t>(status);

    return true;
}

NX_EXPORT bool nxFsWriteS8(PHYSFS_File* handle, int8_t val)
{
    return nxFsWrite(handle, &val, 1u, nullptr) != 0;
}

NX_EXPORT bool nxFsWriteS16(PHYSFS_File* handle, int16_t val)
{
    return PHYSFS_writeSLE16(handle, val) != 0;
}

NX_EXPORT bool nxFsWriteS32(PHYSFS_File* handle, int32_t val)
{
    return PHYSFS_writeSLE32(handle, val) != 0;
}

NX_EXPORT bool nxFsWriteS64(PHYSFS_File* handle, int64_t val)
{
    return PHYSFS_writeSLE64(handle, val) != 0;
}

NX_EXPORT bool nxFsWriteU8(PHYSFS_File* handle, uint8_t val)
{
    return nxFsWrite(handle, &val, 1u, nullptr) != 0;
}

NX_EXPORT bool nxFsWriteU16(PHYSFS_File* handle, uint16_t val)
{
    return PHYSFS_writeULE16(handle, val) != 0;
}

NX_EXPORT bool nxFsWriteU32(PHYSFS_File* handle, uint32_t val)
{
    return PHYSFS_writeULE32(handle, val) != 0;
}

NX_EXPORT bool nxFsWriteU64(PHYSFS_File* handle, uint64_t val)
{
    return PHYSFS_writeULE64(handle, val) != 0;
}

NX_EXPORT bool nxFsWriteFloat(PHYSFS_File* handle, double val)
{
    static_assert(sizeof(float) == 4, "Float is not 4-bytes long");

    // while reinterpret_cast usually works, it can have an undefined behavior at times.
    union
    {
        float f;
        uint32_t u;
    } u = {d2f(val)};

    return nxFsWriteU32(handle, u.u) != 0;
}

NX_EXPORT bool nxFsWriteDouble(PHYSFS_File* handle, double val)
{
    static_assert(sizeof(double) == 8, "Double is not 8-bytes long");

    union
    {
        double d;
        uint64_t u;
    } u = {val};

    return nxFsWriteU64(handle, u.u) != 0;
}

NX_EXPORT bool nxFsWriteString(PHYSFS_File* handle, const char* str)
{
    return nxFsWrite(handle, str, strlen(str) + 1, nullptr) != 0;
}
