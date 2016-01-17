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

// Represents a VRAM texture
class Texture
{
public:
    enum Format : uint8_t {
        Unknown,
        RGBA8,
        DXT1,
        DXT3,
        DXT5,
        RGBA16F,
        RGBA32F,

        PVRTCI_2BPP,
        PVRTCI_A2BPP,
        PVRTCI_4BPP,
        PVRTCI_A4BPP,
        ETC1,

        DEPTH,
        Count
    };

    enum Type : uint8_t{
        _2D,
        Cube
    };

    enum Filter : uint32_t {
        Bilinear  = 0,
        Trilinear = 1,
        Point     = 2,

        _FilterStart = 0,
        _FilterMask  = Bilinear | Trilinear | Point
    };

    enum Anisotropy : uint32_t {
        _1  = 0x00,
        _2  = 0x04,
        _4  = 0x08,
        _8  = 0x10,
        _16 = 0x20,

        _AnisotropyStart = 2,
        _AnisotropyMask = _1 | _2 | _4 | _8 | _16
    };

    enum Repeating : uint32_t {
        ClampX   = 0x000,
        WrapX    = 0x040,
        StretchX = 0x080,
        ClampY   = 0x000,
        WrapY    = 0x100,
        StretchY = 0x200,
        Clamp    = ClampX | ClampY,
        Wrap     = WrapX | WrapY,
        Stretch  = StretchX | StretchY,

        _RepeatingStartX = 6,
        _RepeatingMaskX  = ClampX | WrapX | StretchX,
        _RepeatingStartY = 8,
        _RepeatingMaskY  = ClampY | WrapY | StretchY,
        _RepeatingStart  = 6,
        _RepeatingMask   = Clamp | Wrap | Stretch
    };

    enum Compare : uint32_t {
        LEqual = 0x1000
    };

public:
    virtual ~Texture() = default;

    virtual bool create(Type type, Format format, uint16_t width, uint16_t height,
        bool hasMips, bool mipMaps, bool srgb) = 0;
    virtual void setData(const void* buffer, uint8_t slice, uint8_t level) = 0;
    virtual void setSubData(const void* buffer, uint16_t x, uint16_t y, uint16_t width,
        uint16_t height, uint8_t slice, uint8_t level) = 0;
    virtual bool data(void* buffer, uint8_t slice, uint8_t level) const = 0;
    virtual uint32_t bufferSize() const = 0;

    virtual void size(uint16_t& width, uint16_t& height) const;
    virtual uint16_t width() const = 0;
    virtual uint16_t height() const = 0;

    virtual void setFilter(Filter filter) = 0;
    virtual void setAnisotropyLevel(Anisotropy aniso) = 0;
    virtual void setRepeating(uint32_t repeating) = 0;
    virtual void setLessOrEqual(bool enable) = 0;

    virtual Filter filter() const = 0;
    virtual Anisotropy anisotropyLevel() const = 0;
    virtual uint32_t repeating() const = 0;
    virtual bool lessOrEqual() const = 0;

    virtual bool flipCoords() const = 0;
    virtual Type type() const = 0;
    virtual Format format() const = 0;

    static uint32_t calcSize(Format format, uint32_t width, uint32_t height);
    static uint32_t usedMemory();
    static uint16_t maxSize();
    static void bind(const Texture* texture, uint8_t slot);
};
