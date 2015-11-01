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
#include "renderdevicegl.hpp"
#include "../system/log.hpp"

#if !defined(NX_OPENGL_ES)
//------------------------------------------------------------------------------
#include "opengl.hpp"

//==========================================================
// Locals
//==========================================================
bool extensionsInitialized {false};
#include <iostream>

//----------------------------------------------------------
bool RenderDeviceGL::initialize()
{
    bool failed {false};

    std::string vendor   {(const char*)(glGetString(GL_VENDOR))};
    std::string renderer {(const char*)(glGetString(GL_RENDERER))};
    std::string version  {(const char*)(glGetString(GL_VERSION))};
    Log::info("Initializing OpenGL2 Backend using OpenGL driver '" + version + "'"
        " by '" + vendor + "' on '" + renderer + "'");

    // Initialize extensions
    if (!initOpenGLExtensions()) {
        Log::error("Could not find all required OpenGL function entry points");
        failed = true;
    }
    
    // Check that OpenGL 2.0 is available
    if (glExt::majorVersion < 2) {
        Log::error("OpenGL 2.0 is not available");
        failed = true;
    }

    // Check that required extensions are supported
    if (!glExt::EXT_framebuffer_object) {
        Log::error("Extension EXT_framebuffer_object not supported");
        failed = true;
    }
    if (!glExt::EXT_texture_filter_anisotropic) {
        Log::error("Extension EXT_texture_filter_anisotropic not supported");
        failed = true;
    }
    if (!glExt::EXT_texture_sRGB) {
        Log::error("Extension EXT_texture_sRGB not supported");
        failed = true;
    }

    // Something went wrong
    if (failed) {
        Log::fatal("Failed to init renderer backend, debug info following");

        std::string extensions {reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS))};
        Log::info("Supported OpenGL extensions: '" + extensions + "'");

        return false;
    }

    // Get capabilities
    mCaps.texFloat         = glExt::ARB_texture_float;
    mCaps.texNPOT = glExt::ARB_texture_non_power_of_two;
    mCaps.rtMultisampling = glExt::EXT_framebuffer_multisample;

    // TODO: Find supported depth format (some old ATI cards only support 16 bit depth for FBOs)

    initStates();
    resetStates();

    return true;
}

//----------------------------------------------------------
void RenderDeviceGL::clear(const float* color)
{
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------
void RenderDeviceGL::initStates()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

//----------------------------------------------------------
void RenderDeviceGL::resetStates()
{
    // TODO: Commit new states

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
}

//----------------------------------------------------------
void RenderDeviceGL::beginRendering()
{
    // Get the currently bound frame buffer object. 
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &mDefaultFBO);

    resetStates();
}

//----------------------------------------------------------
void RenderDeviceGL::finishRendering()
{
    // Nothing to do
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::createVertexBuffer(uint32_t size, const void* data)
{
    RDIBuffer buf;

    buf.type = GL_ARRAY_BUFFER;
    buf.size = size;
    glGenBuffers(1, &buf.glObj);
    glBindBuffer(buf.type, buf.glObj);
    glBufferData(buf.type, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(buf.type, 0);

    mBufferMemory += size;
    return mBuffers.add(buf);
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::createIndexBuffer(uint32_t size, const void* data)
{
    RDIBuffer buf;

    buf.type = GL_ELEMENT_ARRAY_BUFFER;
    buf.size = size;
    glGenBuffers(1, &buf.glObj);
    glBindBuffer(buf.type, buf.glObj);
    glBufferData(buf.type, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(buf.type, 0);

    mBufferMemory += size;
    return mBuffers.add(buf);
}

//----------------------------------------------------------
RenderDeviceGL::RDIBuffer& RenderDeviceGL::getBuffer(uint32_t handle)
{
    return mBuffers.getRef(handle);
}

//------------------------------------------------------------------------------
#endif
//==============================================================================
