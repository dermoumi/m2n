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
#include "renderdevice.hpp"

#include "shader.hpp"
#include <memory>
#include <vector>

#ifndef NX_OPENGL_ES
    #include "renderdevicegl.hpp"
#else
    #include "renderdevicegles2.hpp"
#endif

#ifndef NX_GLSL
    #ifndef NX_OPENGL_ES
        #define NX_GLSL(version, shader) "#version " #version "\n" #shader
    #else
        #define NX_GLSL(version, shader) #shader
    #endif
#endif

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
static std::vector<uint32_t> vertexLayouts;
static std::vector<Shader> defaultShaders;

//----------------------------------------------------------
static RenderDevice* getDevice()
{
    #if !defined(NX_OPENGL_ES)
        return new RenderDeviceGL();
    #else
        return new RenderDeviceGLES2();
    #endif
}

//----------------------------------------------------------
RenderDevice& RenderDevice::instance()
{
    static std::unique_ptr<RenderDevice> rdi(getDevice());

    return *rdi;
}

//----------------------------------------------------------
void RenderDevice::release()
{
    // Free up shaders beforehand
    defaultShaders.clear();
}

//----------------------------------------------------------
bool RenderDevice::initialize()
{
    if (!initializeBackend()) return false;


    vertexLayouts.resize(1);
    RenderDevice::VertexLayoutAttrib posTexCoords[2] {
        {"aPosition",  0, 2, 0, 0},
        {"aTexCoords", 0, 2, 8, 0}
    };
    vertexLayouts[0] = RenderDevice::instance().registerVertexLayout(2, posTexCoords);

    defaultShaders.resize(1);
    defaultShaders[0].load(
        NX_GLSL(120,
            attribute vec2 aPosition;
            attribute vec2 aTexCoords;
            uniform mat4 uTransMat;
            varying vec2 vTexCoords;
            void main() {
                vTexCoords  = aTexCoords;
                gl_Position = uTransMat * vec4(aPosition, 0.0, 1.0);
            }
        ),
        NX_GLSL(120,
            uniform sampler2D uTexture;
            uniform vec4 uColor;
            varying vec2 vTexCoords;
            void main() {
                gl_FragColor = texture2D(uTexture, vTexCoords) * uColor;
            }
        )
    );

    return true;
}

//----------------------------------------------------------
uint32_t RenderDevice::vertexLayout(uint32_t index)
{
    if (index > vertexLayouts.size()) return 0;
    return vertexLayouts[index];
}

//----------------------------------------------------------
Shader* RenderDevice::defaultShader(uint32_t index)
{
    if (index > defaultShaders.size()) return nullptr;
    return &defaultShaders[index];
}

//----------------------------------------------------------
uint32_t RenderDevice::calcTextureSize(TextureFormat format, int width, int height, int depth)
{
    switch(format)
    {
    case RGBA8:
        return width * height * depth * 4;
    case DXT1:
        return std::max(width / 4, 1) * std::max(height / 4, 1) * depth * 8;
    case DXT3:
        return std::max(width / 4, 1) * std::max(height / 4, 1) * depth * 16;
    case DXT5:
        return std::max(width / 4, 1) * std::max(height / 4, 1) * depth * 16;
    case RGBA16F:
        return width * height * depth * 8;
    case RGBA32F:
        return width * height * depth * 16;
    case PVRTCI_2BPP:
    case PVRTCI_A2BPP:
        return (std::max(width, 16) * std::max(height, 8) * 2 + 7) / 8;
    case PVRTCI_4BPP:
    case PVRTCI_A4BPP:
        return (std::max(width, 8) * std::max(height, 8) * 4 + 7) / 8;
    case ETC1:
        return std::max(width / 4, 1) * std::max(height / 4, 1) * depth * 8;
    default:
        return 0;
    }
}

//==============================================================================