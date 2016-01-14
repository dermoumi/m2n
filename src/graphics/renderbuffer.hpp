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
#include "texture.hpp"

#include <memory>

// Represents a drawable texture in the VRAM
class RenderBuffer
{
public:
    virtual ~RenderBuffer() = default;

    virtual bool create(uint8_t format, uint16_t width, uint16_t height, bool depth,
        uint8_t colBufCount, uint8_t samples) = 0;
    virtual Texture* texture(uint8_t index) = 0;

    virtual void size(uint16_t& w, uint16_t& h) const;
    virtual uint16_t width() const = 0;
    virtual uint16_t height() const = 0;
    virtual uint8_t format() const = 0;

    static void bind(RenderBuffer* buffer);
};
