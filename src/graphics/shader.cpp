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
#include "shader.hpp"
#include "renderdevice.hpp"
    
//----------------------------------------------------------
Shader::~Shader()
{
    if (mHandle) {
        RenderDevice::instance().destroyShader(mHandle);
    }
}

//----------------------------------------------------------
bool Shader::load(const char* vertexShader, const char* fragmentShader)
{
    if (mHandle) RenderDevice::instance().destroyShader(mHandle);

    mHandle = RenderDevice::instance().createShader(vertexShader, fragmentShader);
    return mHandle != 0;
}

//----------------------------------------------------------
void Shader::setUniform(int location, uint8_t type, float* data)
{
    auto prevShader = RenderDevice::instance().getCurrentShader();
    RenderDevice::instance().bindShader(mHandle);

    RenderDevice::instance().setShaderConst(
        location, static_cast<RenderDevice::ShaderConstType>(type), data, 1
    );

    if (prevShader != mHandle) RenderDevice::instance().bindShader(prevShader);
}

//----------------------------------------------------------
void Shader::setSampler(int location, uint8_t unit)
{
    auto prevShader = RenderDevice::instance().getCurrentShader();
    RenderDevice::instance().bindShader(mHandle);

    RenderDevice::instance().setShaderSampler(location, unit);

    if (prevShader != mHandle) RenderDevice::instance().bindShader(prevShader);
}

//----------------------------------------------------------
int Shader::uniformLocation(const char* name) const
{
    return RenderDevice::instance().getShaderConstLoc(mHandle, name);
}

//----------------------------------------------------------
int Shader::samplerLocation(const char* name) const
{
    return RenderDevice::instance().getShaderSamplerLoc(mHandle, name);
}

//----------------------------------------------------------
const char* Shader::log()
{
    return RenderDevice::instance().getShaderLog().data();
}

//----------------------------------------------------------
void Shader::bind(const Shader* shader)
{
    RenderDevice::instance().bindShader(shader ? shader->mHandle : 0);
}

//----------------------------------------------------------
const char* Shader::defaultVSCode()
{
    return RenderDevice::instance().getDefaultVSCode();
}

//----------------------------------------------------------
const char* Shader::defaultFSCode()
{
    return RenderDevice::instance().getDefaultFSCode();
}
