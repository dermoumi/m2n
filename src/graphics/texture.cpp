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

#include "texture.hpp"
#include "renderdevice.hpp"

#include <algorithm>

static uint16_t maximumSize()
{
    unsigned int maxSize;
    RenderDevice::instance().getCapabilities(nullptr, &maxSize, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr);
    return maxSize;
}

void Texture::size(uint16_t& w, uint16_t& h, uint16_t& d) const
{
    w = width();
    h = height();
    d = depth();
}

uint32_t Texture::calcSize(Format format, uint32_t width, uint32_t height, uint32_t depth)
{
    switch(format)
    {
    case RGBA8:
        return width * height * depth * 4;
    case DXT1:
        return std::max(width / 4, 1u) * std::max(height / 4u, 1u) * depth * 8;
    case DXT3:
        return std::max(width / 4, 1u) * std::max(height / 4u, 1u) * depth * 16;
    case DXT5:
        return std::max(width / 4, 1u) * std::max(height / 4u, 1u) * depth * 16;
    case RGBA16F:
        return width * height * depth * 8;
    case RGBA32F:
        return width * height * depth * 16;
    case PVRTCI_2BPP:
    case PVRTCI_A2BPP:
        return (std::max(width, 16u) * std::max(height, 8u) * 2 + 7) / 8;
    case PVRTCI_4BPP:
    case PVRTCI_A4BPP:
        return (std::max(width, 8u) * std::max(height, 8u) * 4 + 7) / 8;
    case ETC1:
        return std::max(width / 4, 1u) * std::max(height / 4, 1u) * depth * 8;
    default:
        return 0u;
    }
}

uint32_t Texture::usedMemory()
{
    return RenderDevice::instance().usedTextureMemory();
}

uint16_t Texture::maxSize()
{
    static uint16_t max = maximumSize();
    return max;
}

void Texture::bind(const Texture* texture, uint8_t slot)
{
    RenderDevice::instance().bind(texture, slot);
}
