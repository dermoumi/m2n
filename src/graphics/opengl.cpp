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
#define _CRT_SECURE_NO_WARNINGS
#include "opengl.hpp"

//==============================================================================
#if !defined(NX_OPENGL_ES)
//------------------------------------------------------------------------------

#include <cstdlib>
#include <cstring>
#include <string>

#include <SDL2/SDL.h>

//----------------------------------------------------------
// Capabilities
//----------------------------------------------------------
namespace glExt
{
    bool EXT_framebuffer_object = false;
    bool EXT_framebuffer_multisample = false;
    bool EXT_texture_filter_anisotropic = false;
    bool EXT_texture_compression_s3tc = false;
    bool EXT_texture_sRGB = false;
    bool ARB_texture_float = false;
    bool ARB_texture_non_power_of_two = false;
    bool ARB_timer_query = false;

    int majorVersion = 1, minorVersion = 0;
}

//----------------------------------------------------------
// OpenGL Functions
//----------------------------------------------------------
// GL 1.2
PFNGLBLENDCOLORPROC glBlendColor = 0x0;
PFNGLBLENDEQUATIONPROC glBlendEquation = 0x0;
PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements = 0x0;
PFNGLTEXIMAGE3DPROC glTexImage3D = 0x0;
PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = 0x0;
PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D = 0x0;

// GL 1.3
PFNGLACTIVETEXTUREPROC glActiveTexture = 0x0;
PFNGLSAMPLECOVERAGEPROC glSampleCoverage = 0x0;
PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D = 0x0;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D = 0x0;
PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D = 0x0;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D = 0x0;
PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage = 0x0;

// GL 1.4
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = 0x0;
PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays = 0x0;
PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements = 0x0;
PFNGLPOINTPARAMETERFPROC glPointParameterf = 0x0;
PFNGLPOINTPARAMETERFVPROC glPointParameterfv = 0x0;
PFNGLPOINTPARAMETERIPROC glPointParameteri = 0x0;
PFNGLPOINTPARAMETERIVPROC glPointParameteriv = 0x0;

// GL 1.5
PFNGLGENQUERIESPROC glGenQueries = 0x0;
PFNGLDELETEQUERIESPROC glDeleteQueries = 0x0;
PFNGLISQUERYPROC glIsQuery = 0x0;
PFNGLBEGINQUERYPROC glBeginQuery = 0x0;
PFNGLENDQUERYPROC glEndQuery = 0x0;
PFNGLGETQUERYIVPROC glGetQueryiv = 0x0;
PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv = 0x0;
PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv = 0x0;
PFNGLBINDBUFFERPROC glBindBuffer = 0x0;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = 0x0;
PFNGLGENBUFFERSPROC glGenBuffers = 0x0;
PFNGLISBUFFERPROC glIsBuffer = 0x0;
PFNGLBUFFERDATAPROC glBufferData = 0x0;
PFNGLBUFFERSUBDATAPROC glBufferSubData = 0x0;
PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData = 0x0;
PFNGLMAPBUFFERPROC glMapBuffer = 0x0;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = 0x0;
PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv = 0x0;
PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv = 0x0;

// GL 2.0
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = 0x0;
PFNGLDRAWBUFFERSPROC glDrawBuffers = 0x0;
PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate = 0x0;
PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate = 0x0;
PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate = 0x0;
PFNGLATTACHSHADERPROC glAttachShader = 0x0;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = 0x0;
PFNGLCOMPILESHADERPROC glCompileShader = 0x0;
PFNGLCREATEPROGRAMPROC glCreateProgram = 0x0;
PFNGLCREATESHADERPROC glCreateShader = 0x0;
PFNGLDELETEPROGRAMPROC glDeleteProgram = 0x0;
PFNGLDELETESHADERPROC glDeleteShader = 0x0;
PFNGLDETACHSHADERPROC glDetachShader = 0x0;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = 0x0;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = 0x0;
PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib = 0x0;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = 0x0;
PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders = 0x0;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = 0x0;
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0x0;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0x0;
PFNGLGETSHADERIVPROC glGetShaderiv = 0x0;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = 0x0;
PFNGLGETSHADERSOURCEPROC glGetShaderSource = 0x0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = 0x0;
PFNGLGETUNIFORMFVPROC glGetUniformfv = 0x0;
PFNGLGETUNIFORMIVPROC glGetUniformiv = 0x0;
PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv = 0x0;
PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv = 0x0;
PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv = 0x0;
PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv = 0x0;
PFNGLISPROGRAMPROC glIsProgram = 0x0;
PFNGLISSHADERPROC glIsShader = 0x0;
PFNGLLINKPROGRAMPROC glLinkProgram = 0x0;
PFNGLSHADERSOURCEPROC glShaderSource = 0x0;
PFNGLUSEPROGRAMPROC glUseProgram = 0x0;
PFNGLUNIFORM1FPROC glUniform1f = 0x0;
PFNGLUNIFORM2FPROC glUniform2f = 0x0;
PFNGLUNIFORM3FPROC glUniform3f = 0x0;
PFNGLUNIFORM4FPROC glUniform4f = 0x0;
PFNGLUNIFORM1IPROC glUniform1i = 0x0;
PFNGLUNIFORM2IPROC glUniform2i = 0x0;
PFNGLUNIFORM3IPROC glUniform3i = 0x0;
PFNGLUNIFORM4IPROC glUniform4i = 0x0;
PFNGLUNIFORM1FVPROC glUniform1fv = 0x0;
PFNGLUNIFORM2FVPROC glUniform2fv = 0x0;
PFNGLUNIFORM3FVPROC glUniform3fv = 0x0;
PFNGLUNIFORM4FVPROC glUniform4fv = 0x0;
PFNGLUNIFORM1IVPROC glUniform1iv = 0x0;
PFNGLUNIFORM2IVPROC glUniform2iv = 0x0;
PFNGLUNIFORM3IVPROC glUniform3iv = 0x0;
PFNGLUNIFORM4IVPROC glUniform4iv = 0x0;
PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv = 0x0;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv = 0x0;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = 0x0;
PFNGLVALIDATEPROGRAMPROC glValidateProgram = 0x0;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = 0x0;

// GL 2.1
PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv = 0x0;
PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv = 0x0;
PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv = 0x0;
PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv = 0x0;
PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv = 0x0;
PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv = 0x0;

// GL 3.0
PFNGLGETSTRINGIPROC glGetStringi = 0x0;

// GL_EXT_framebuffer_object
PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT = 0x0;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = 0x0;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = 0x0;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = 0x0;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = 0x0;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT = 0x0;
PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT = 0x0;
PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = 0x0;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = 0x0;
PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = 0x0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = 0x0;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT = 0x0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = 0x0;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT = 0x0;
PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = 0x0;

// GL_EXT_framebuffer_blit
PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT = 0x0;

// GL_EXT_framebuffer_multisample
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT = 0x0;

// GL_ARB_timer_query
PFNGLQUERYCOUNTERPROC glQueryCounter = 0x0;
PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v = 0x0;
PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v = 0x0;

//----------------------------------------------------------
// Local
//----------------------------------------------------------
namespace
{
    NX_HIDDEN bool isExtensionSupported(const std::string& extName)
    {
        if(glExt::majorVersion < 3)
        {
            static std::string extensions = (char*)glGetString(GL_EXTENSIONS) + std::string(" ");
            return extensions.find(extName + " ") != std::string::npos;
        }
        else
        {
            GLint numExts;
            glGetIntegerv(GL_NUM_EXTENSIONS, &numExts);
            for ( int i = 0; i < numExts; ++i )
            {
                if (extName == (char *)glGetStringi(GL_EXTENSIONS, i)) return true;
            }
        }

        return false;
    }

    NX_HIDDEN void getOpenGLVersion()
    {
        // Retrieve the actual context version
        int major {0};
        int minor {0};
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        if (glGetError() != GL_INVALID_ENUM) {
            glExt::majorVersion = static_cast<unsigned int>(major);
            glExt::minorVersion = static_cast<unsigned int>(minor);
        }
        else {
            // Try the "old" way
            const GLubyte* version {glGetString(GL_VERSION)};
            if (version) {
                // The returned string starts with major.minor (it's a standard)
                glExt::majorVersion = version[0] - '0';
                glExt::minorVersion = version[2] - '0';
            }
            else {
                // Can't get the version number, assume 1.1
                glExt::majorVersion = 1;
                glExt::minorVersion = 1;
            }
        }
    }
}

//----------------------------------------------------------
// Initialization function
//----------------------------------------------------------
bool initOpenGLExtensions()
{
    bool r = true;
    
    getOpenGLVersion();
    
    // GL 1.2
    r &= (glBlendColor = (PFNGLBLENDCOLORPROC) SDL_GL_GetProcAddress("glBlendColor")) != nullptr;
    r &= (glBlendEquation = (PFNGLBLENDEQUATIONPROC) SDL_GL_GetProcAddress("glBlendEquation")) != nullptr;
    r &= (glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC) SDL_GL_GetProcAddress("glDrawRangeElements")) != nullptr;
    r &= (glTexImage3D = (PFNGLTEXIMAGE3DPROC) SDL_GL_GetProcAddress("glTexImage3D")) != nullptr;
    r &= (glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC) SDL_GL_GetProcAddress("glTexSubImage3D")) != nullptr;
    r &= (glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC) SDL_GL_GetProcAddress("glCopyTexSubImage3D")) != nullptr;

    // GL 1.3
    r &= (glActiveTexture = (PFNGLACTIVETEXTUREPROC) SDL_GL_GetProcAddress("glActiveTexture")) != nullptr;
    r &= (glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) SDL_GL_GetProcAddress("glSampleCoverage")) != nullptr;
    r &= (glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC) SDL_GL_GetProcAddress("glCompressedTexImage3D")) != nullptr;
    r &= (glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SDL_GL_GetProcAddress("glCompressedTexImage2D")) != nullptr;
    r &= (glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC) SDL_GL_GetProcAddress("glCompressedTexImage1D")) != nullptr;
    r &= (glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) SDL_GL_GetProcAddress("glCompressedTexSubImage3D")) != nullptr;
    r &= (glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) SDL_GL_GetProcAddress("glCompressedTexSubImage2D")) != nullptr;
    r &= (glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) SDL_GL_GetProcAddress("glCompressedTexSubImage1D")) != nullptr;
    r &= (glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC) SDL_GL_GetProcAddress("glGetCompressedTexImage")) != nullptr;
    
    // GL 1.4
    r &= (glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SDL_GL_GetProcAddress("glBlendFuncSeparate")) != nullptr;
    r &= (glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC) SDL_GL_GetProcAddress("glMultiDrawArrays")) != nullptr;
    r &= (glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC) SDL_GL_GetProcAddress("glMultiDrawElements")) != nullptr;
    r &= (glPointParameterf = (PFNGLPOINTPARAMETERFPROC) SDL_GL_GetProcAddress("glPointParameterf")) != nullptr;
    r &= (glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC) SDL_GL_GetProcAddress("glPointParameterfv")) != nullptr;
    r &= (glPointParameteri = (PFNGLPOINTPARAMETERIPROC) SDL_GL_GetProcAddress("glPointParameteri")) != nullptr;
    r &= (glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC) SDL_GL_GetProcAddress("glPointParameteriv")) != nullptr;

    // GL 1.5
    r &= (glGenQueries = (PFNGLGENQUERIESPROC) SDL_GL_GetProcAddress("glGenQueries")) != nullptr;
    r &= (glDeleteQueries = (PFNGLDELETEQUERIESPROC) SDL_GL_GetProcAddress("glDeleteQueries")) != nullptr;
    r &= (glIsQuery = (PFNGLISQUERYPROC) SDL_GL_GetProcAddress("glIsQuery")) != nullptr;
    r &= (glBeginQuery = (PFNGLBEGINQUERYPROC) SDL_GL_GetProcAddress("glBeginQuery")) != nullptr;
    r &= (glEndQuery = (PFNGLENDQUERYPROC) SDL_GL_GetProcAddress("glEndQuery")) != nullptr;
    r &= (glGetQueryiv = (PFNGLGETQUERYIVPROC) SDL_GL_GetProcAddress("glGetQueryiv")) != nullptr;
    r &= (glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC) SDL_GL_GetProcAddress("glGetQueryObjectiv")) != nullptr;
    r &= (glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC) SDL_GL_GetProcAddress("glGetQueryObjectuiv")) != nullptr;
    r &= (glBindBuffer = (PFNGLBINDBUFFERPROC) SDL_GL_GetProcAddress("glBindBuffer")) != nullptr;
    r &= (glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SDL_GL_GetProcAddress("glDeleteBuffers")) != nullptr;
    r &= (glGenBuffers = (PFNGLGENBUFFERSPROC) SDL_GL_GetProcAddress("glGenBuffers")) != nullptr;
    r &= (glIsBuffer = (PFNGLISBUFFERPROC) SDL_GL_GetProcAddress("glIsBuffer")) != nullptr;
    r &= (glBufferData = (PFNGLBUFFERDATAPROC) SDL_GL_GetProcAddress("glBufferData")) != nullptr;
    r &= (glBufferSubData = (PFNGLBUFFERSUBDATAPROC) SDL_GL_GetProcAddress("glBufferSubData")) != nullptr;
    r &= (glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC) SDL_GL_GetProcAddress("glGetBufferSubData")) != nullptr;
    r &= (glMapBuffer = (PFNGLMAPBUFFERPROC) SDL_GL_GetProcAddress("glMapBuffer")) != nullptr;
    r &= (glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SDL_GL_GetProcAddress("glUnmapBuffer")) != nullptr;
    r &= (glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) SDL_GL_GetProcAddress("glGetBufferParameteriv")) != nullptr;
    r &= (glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC) SDL_GL_GetProcAddress("glGetBufferPointerv")) != nullptr;
    
    // GL 2.0
    r &= (glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) SDL_GL_GetProcAddress("glBlendEquationSeparate")) != nullptr;
    r &= (glDrawBuffers = (PFNGLDRAWBUFFERSPROC) SDL_GL_GetProcAddress("glDrawBuffers")) != nullptr;
    r &= (glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) SDL_GL_GetProcAddress("glStencilOpSeparate")) != nullptr;
    r &= (glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) SDL_GL_GetProcAddress("glStencilFuncSeparate")) != nullptr;
    r &= (glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC) SDL_GL_GetProcAddress("glStencilMaskSeparate")) != nullptr;
    r &= (glAttachShader = (PFNGLATTACHSHADERPROC) SDL_GL_GetProcAddress("glAttachShader")) != nullptr;
    r &= (glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) SDL_GL_GetProcAddress("glBindAttribLocation")) != nullptr;
    r &= (glCompileShader = (PFNGLCOMPILESHADERPROC) SDL_GL_GetProcAddress("glCompileShader")) != nullptr;
    r &= (glCreateProgram = (PFNGLCREATEPROGRAMPROC) SDL_GL_GetProcAddress("glCreateProgram")) != nullptr;
    r &= (glCreateShader = (PFNGLCREATESHADERPROC) SDL_GL_GetProcAddress("glCreateShader")) != nullptr;
    r &= (glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SDL_GL_GetProcAddress("glDeleteProgram")) != nullptr;
    r &= (glDeleteShader = (PFNGLDELETESHADERPROC) SDL_GL_GetProcAddress("glDeleteShader")) != nullptr;
    r &= (glDetachShader = (PFNGLDETACHSHADERPROC) SDL_GL_GetProcAddress("glDetachShader")) != nullptr;
    r &= (glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SDL_GL_GetProcAddress("glDisableVertexAttribArray")) != nullptr;
    r &= (glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SDL_GL_GetProcAddress("glEnableVertexAttribArray")) != nullptr;
    r &= (glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) SDL_GL_GetProcAddress("glGetActiveAttrib")) != nullptr;
    r &= (glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) SDL_GL_GetProcAddress("glGetActiveUniform")) != nullptr;
    r &= (glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) SDL_GL_GetProcAddress("glGetAttachedShaders")) != nullptr;
    r &= (glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) SDL_GL_GetProcAddress("glGetAttribLocation")) != nullptr;
    r &= (glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SDL_GL_GetProcAddress("glGetProgramiv")) != nullptr;
    r &= (glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SDL_GL_GetProcAddress("glGetProgramInfoLog")) != nullptr;
    r &= (glGetShaderiv = (PFNGLGETSHADERIVPROC) SDL_GL_GetProcAddress("glGetShaderiv")) != nullptr;
    r &= (glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SDL_GL_GetProcAddress("glGetShaderInfoLog")) != nullptr;
    r &= (glGetShaderSource = (PFNGLGETSHADERSOURCEPROC) SDL_GL_GetProcAddress("glGetShaderSource")) != nullptr;
    r &= (glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SDL_GL_GetProcAddress("glGetUniformLocation")) != nullptr;
    r &= (glGetUniformfv = (PFNGLGETUNIFORMFVPROC) SDL_GL_GetProcAddress("glGetUniformfv")) != nullptr;
    r &= (glGetUniformiv = (PFNGLGETUNIFORMIVPROC) SDL_GL_GetProcAddress("glGetUniformiv")) != nullptr;
    r &= (glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC) SDL_GL_GetProcAddress("glGetVertexAttribdv")) != nullptr;
    r &= (glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) SDL_GL_GetProcAddress("glGetVertexAttribfv")) != nullptr;
    r &= (glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC) SDL_GL_GetProcAddress("glGetVertexAttribiv")) != nullptr;
    r &= (glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC) SDL_GL_GetProcAddress("glGetVertexAttribPointerv")) != nullptr;
    r &= (glIsProgram = (PFNGLISPROGRAMPROC) SDL_GL_GetProcAddress("glIsProgram")) != nullptr;
    r &= (glIsShader = (PFNGLISSHADERPROC) SDL_GL_GetProcAddress("glIsShader")) != nullptr;
    r &= (glLinkProgram = (PFNGLLINKPROGRAMPROC) SDL_GL_GetProcAddress("glLinkProgram")) != nullptr;
    r &= (glShaderSource = (PFNGLSHADERSOURCEPROC) SDL_GL_GetProcAddress("glShaderSource")) != nullptr;
    r &= (glUseProgram = (PFNGLUSEPROGRAMPROC) SDL_GL_GetProcAddress("glUseProgram")) != nullptr;
    r &= (glUniform1f = (PFNGLUNIFORM1FPROC) SDL_GL_GetProcAddress("glUniform1f")) != nullptr;
    r &= (glUniform2f = (PFNGLUNIFORM2FPROC) SDL_GL_GetProcAddress("glUniform2f")) != nullptr;
    r &= (glUniform3f = (PFNGLUNIFORM3FPROC) SDL_GL_GetProcAddress("glUniform3f")) != nullptr;
    r &= (glUniform4f = (PFNGLUNIFORM4FPROC) SDL_GL_GetProcAddress("glUniform4f")) != nullptr;
    r &= (glUniform1i = (PFNGLUNIFORM1IPROC) SDL_GL_GetProcAddress("glUniform1i")) != nullptr;
    r &= (glUniform2i = (PFNGLUNIFORM2IPROC) SDL_GL_GetProcAddress("glUniform2i")) != nullptr;
    r &= (glUniform3i = (PFNGLUNIFORM3IPROC) SDL_GL_GetProcAddress("glUniform3i")) != nullptr;
    r &= (glUniform4i = (PFNGLUNIFORM4IPROC) SDL_GL_GetProcAddress("glUniform4i")) != nullptr;
    r &= (glUniform1fv = (PFNGLUNIFORM1FVPROC) SDL_GL_GetProcAddress("glUniform1fv")) != nullptr;
    r &= (glUniform2fv = (PFNGLUNIFORM2FVPROC) SDL_GL_GetProcAddress("glUniform2fv")) != nullptr;
    r &= (glUniform3fv = (PFNGLUNIFORM3FVPROC) SDL_GL_GetProcAddress("glUniform3fv")) != nullptr;
    r &= (glUniform4fv = (PFNGLUNIFORM4FVPROC) SDL_GL_GetProcAddress("glUniform4fv")) != nullptr;
    r &= (glUniform1iv = (PFNGLUNIFORM1IVPROC) SDL_GL_GetProcAddress("glUniform1iv")) != nullptr;
    r &= (glUniform2iv = (PFNGLUNIFORM2IVPROC) SDL_GL_GetProcAddress("glUniform2iv")) != nullptr;
    r &= (glUniform3iv = (PFNGLUNIFORM3IVPROC) SDL_GL_GetProcAddress("glUniform3iv")) != nullptr;
    r &= (glUniform4iv = (PFNGLUNIFORM4IVPROC) SDL_GL_GetProcAddress("glUniform4iv")) != nullptr;
    r &= (glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) SDL_GL_GetProcAddress("glUniformMatrix2fv")) != nullptr;
    r &= (glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) SDL_GL_GetProcAddress("glUniformMatrix3fv")) != nullptr;
    r &= (glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SDL_GL_GetProcAddress("glUniformMatrix4fv")) != nullptr;
    r &= (glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) SDL_GL_GetProcAddress("glValidateProgram")) != nullptr;
    r &= (glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SDL_GL_GetProcAddress("glVertexAttribPointer")) != nullptr;

    /*// GL 2.1
    r &= (glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC) SDL_GL_GetProcAddress("glUniformMatrix2x3fv")) != nullptr;
    r &= (glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC) SDL_GL_GetProcAddress("glUniformMatrix3x2fv")) != nullptr;
    r &= (glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC) SDL_GL_GetProcAddress("glUniformMatrix2x4fv")) != nullptr;
    r &= (glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC) SDL_GL_GetProcAddress("glUniformMatrix4x2fv")) != nullptr;
    r &= (glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC) SDL_GL_GetProcAddress("glUniformMatrix3x4fv")) != nullptr;
    r &= (glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC) SDL_GL_GetProcAddress("glUniformMatrix4x3fv")) != nullptr;
    */

    // GL 3.0 
    if (glExt::majorVersion >= 3) {
        r &= (glGetStringi = (PFNGLGETSTRINGIPROC) SDL_GL_GetProcAddress("glGetStringi")) != nullptr;
    }
    
    // Extensions
    glExt::EXT_framebuffer_object = isExtensionSupported("GL_EXT_framebuffer_object");
    if (glExt::EXT_framebuffer_object)
    {
        r &= (glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC) SDL_GL_GetProcAddress("glIsRenderbufferEXT")) != nullptr;
        r &= (glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) SDL_GL_GetProcAddress("glBindRenderbufferEXT")) != nullptr;
        r &= (glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT")) != nullptr;
        r &= (glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) SDL_GL_GetProcAddress("glGenRenderbuffersEXT")) != nullptr;
        r &= (glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) SDL_GL_GetProcAddress("glRenderbufferStorageEXT")) != nullptr;
        r &= (glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) SDL_GL_GetProcAddress("glGetRenderbufferParameterivEXT")) != nullptr;
        r &= (glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC) SDL_GL_GetProcAddress("glIsFramebufferEXT")) != nullptr;
        r &= (glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) SDL_GL_GetProcAddress("glBindFramebufferEXT")) != nullptr;
        r &= (glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) SDL_GL_GetProcAddress("glDeleteFramebuffersEXT")) != nullptr;
        r &= (glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) SDL_GL_GetProcAddress("glGenFramebuffersEXT")) != nullptr;
        r &= (glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT")) != nullptr;
        r &= (glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) SDL_GL_GetProcAddress("glFramebufferTexture1DEXT")) != nullptr;
        r &= (glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) SDL_GL_GetProcAddress("glFramebufferTexture2DEXT")) != nullptr;
        r &= (glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) SDL_GL_GetProcAddress("glFramebufferTexture3DEXT")) != nullptr;
        r &= (glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT")) != nullptr;
        r &= (glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) SDL_GL_GetProcAddress("glGetFramebufferAttachmentParameterivEXT")) != nullptr;
        r &= (glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) SDL_GL_GetProcAddress("glGenerateMipmapEXT")) != nullptr;
    }
    
    glExt::EXT_texture_filter_anisotropic = isExtensionSupported("GL_EXT_texture_filter_anisotropic");

    glExt::EXT_texture_compression_s3tc = isExtensionSupported("GL_EXT_texture_compression_s3tc");

    glExt::EXT_texture_sRGB = isExtensionSupported("GL_EXT_texture_sRGB");

    glExt::ARB_texture_float = isExtensionSupported("GL_ARB_texture_float") ||
                               isExtensionSupported("GL_ATI_texture_float");

    glExt::ARB_texture_non_power_of_two = isExtensionSupported("GL_ARB_texture_non_power_of_two");

    glExt::EXT_framebuffer_multisample = isExtensionSupported("GL_EXT_framebuffer_multisample") &&
                                         isExtensionSupported("GL_EXT_framebuffer_blit");
    if (glExt::EXT_framebuffer_multisample) {
        // From GL_EXT_framebuffer_blit
        r &= (glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC) SDL_GL_GetProcAddress("glBlitFramebufferEXT")) != nullptr;
        // From GL_EXT_framebuffer_multisample
        r &= (glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) SDL_GL_GetProcAddress("glRenderbufferStorageMultisampleEXT")) != nullptr;
    }

    glExt::ARB_timer_query = isExtensionSupported("GL_ARB_timer_query");
    if (glExt::ARB_timer_query) {
        r &= (glQueryCounter = (PFNGLQUERYCOUNTERPROC) SDL_GL_GetProcAddress("glQueryCounter")) != nullptr;
        r &= (glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC) SDL_GL_GetProcAddress("glGetQueryObjecti64v")) != nullptr;
        r &= (glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC) SDL_GL_GetProcAddress("glGetQueryObjectui64v")) != nullptr;
    }

    return r;
}

//------------------------------------------------------------------------------
#endif
//==============================================================================
