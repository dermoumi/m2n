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
#include "../graphics/shader.hpp"

using NxShader = Shader;

NX_EXPORT NxShader* nxShaderNew()
{
    return new Shader();
}

NX_EXPORT void nxShaderRelease(NxShader* shader)
{
    delete shader;
}

NX_EXPORT bool nxShaderLoad(NxShader* shader, const char* vertexShader, const char* fragmentShader)
{
    return shader->load(vertexShader, fragmentShader);
}

NX_EXPORT void nxShaderSetUniform(NxShader* shader, int loc, uint8_t type, float* data)
{
    return shader->setUniform(loc, type, data);
}

NX_EXPORT void nxShaderSetSampler(NxShader* shader, int loc, int unit)
{
    return shader->setSampler(loc, unit);
}

NX_EXPORT int nxShaderUniformLocation(const NxShader* shader, const char* name)
{
    return shader->uniformLocation(name);
}

NX_EXPORT int nxShaderSamplerLocation(const NxShader* shader, const char* name)
{
    return shader->samplerLocation(name);
}

NX_EXPORT const char* nxShaderLog()
{
    return Shader::log();
}

NX_EXPORT void nxShaderBind(const NxShader* shader)
{
    Shader::bind(shader);
}

NX_EXPORT const char* nxShaderDefaultVSCode()
{
    return Shader::defaultVSCode();
}

NX_EXPORT const char* nxShaderDefaultFSCode()
{
    return Shader::defaultFSCode();
}
