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
    OTHER LIABILITY, WHETHER IN AN CATION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org>
*/

#pragma once
#include "../config.hpp"

// Represents a set of GLSL Vertex and Fragment shaders
class Shader
{
public:
    virtual ~Shader() = default;

    virtual bool load(const char* vertexShader, const char* fragmentShader) = 0;
    virtual void setUniform(int location, uint8_t type, float* data, uint32_t count = 1) = 0;
    virtual void setSampler(int location, uint8_t unit) = 0;

    virtual int uniformLocation(const char* name) const = 0;
    virtual int samplerLocation(const char* name) const = 0;

    static const char* log();
    static const char* defaultVSCode();
    static const char* defaultFSCode();
    static void bind(Shader* shader);
};
