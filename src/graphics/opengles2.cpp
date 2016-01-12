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

#define _CRT_SECURE_NO_WARNINGS
#include "opengles2.hpp"

#if defined(NX_OPENGL_ES)

#include <cstdlib>
#include <cstring>
#include <string>

#include <SDL2/SDL.h>

#if defined(NX_SYSTEM_WINDOWS) || defined(NX_SYSTEM_ANDROID)
    #include "EGL/egl.h"
#elif defined(NX_SYSTEM_IOS)
    #include <CoreFoundation/CoreFoundation.h>
#endif

// Capabilities
namespace glExt
{
    bool EXT_multisampled_render_to_texture = false;

    bool ANGLE_framebuffer_blit = false;
    bool ANGLE_framebuffer_multisample = false;
    bool OES_rgb8_rgba8 = false;

    bool EXT_texture_filter_anisotropic = false;
    bool EXT_disjoint_timer_query = false;
    bool EXT_occlusion_query_boolean = false;

    bool OES_texture_3D = false;

    bool EXT_texture_compression_s3tc = false;
    bool EXT_texture_compression_dxt1 = false;
    bool ANGLE_texture_compression_dxt3 = false;
    bool ANGLE_texture_compression_dxt5 = false;

    bool IMG_texture_compression_pvrtc = false;
    bool OES_compressed_ETC1_RGB8_texture = false;

    bool OES_depth_texture = false;
    bool ANGLE_depth_texture = false;

    bool EXT_shadow_samplers = false;

    int majorVersion = 1, minorVersion = 0;
}

// Extensions
#ifdef NXGL_OES_texture_3D
    PFNGLTEXIMAGE3DOESPROC glTexImage3DOES = 0x0;
    PFNGLTEXSUBIMAGE3DOESPROC glTexSubImage3DOES = 0x0;
    PFNGLCOPYTEXSUBIMAGE3DOESPROC glCopyTexSubImage3DOES = 0x0;
    PFNGLCOMPRESSEDTEXIMAGE3DOESPROC glCompressedTexImage3DOES = 0x0;
    PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC glCompressedTexSubImage3DOES = 0x0;
#endif

#ifdef NXGL_EXT_multisampled_render_to_texture
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT = 0x0;
    PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT = 0x0;
#endif

#ifdef NXGL_ANGLE_framebuffer_blit
    PFNGLBLITFRAMEBUFFERANGLEPROC glBlitFramebufferANGLE = 0x0;
#endif

#ifdef NXGL_ANGLE_framebuffer_multisample
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC glRenderbufferStorageMultisampleANGLE = 0x0;
#endif

#ifdef NXGL_EXT_occlusion_query_boolean
    PFNGLGENQUERIESEXTPROC glGenQueriesEXT = 0x0;
    PFNGLDELETEQUERIESEXTPROC glDeleteQueriesEXT = 0x0;
    PFNGLISQUERYEXTPROC glIsQueryEXT = 0x0;
    PFNGLBEGINQUERYEXTPROC glBeginQueryEXT = 0x0;
    PFNGLENDQUERYEXTPROC glEndQueryEXT = 0x0;
    PFNGLGETQUERYIVEXTPROC glGetQueryivEXT = 0x0;
    PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuivEXT = 0x0;
#endif

// Locals
namespace
{
    NX_HIDDEN bool isExtensionSupported(const std::string& extName)
    {
        static std::string extensions = (char*)glGetString(GL_EXTENSIONS) + std::string(" ");
        return extensions.find(extName + " ") != std::string::npos;
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

NX_HIDDEN bool initOpenGLExtensions()
{
    getOpenGLVersion();

    // Extensions
    glExt::OES_texture_3D = isExtensionSupported("GL_OES_texture_3D");
    #ifdef NXGL_OES_texture_3D
        if (glExt::OES_texture_3D) {
            bool v = true;
            v &= (glTexImage3DOES = (PFNGLTEXIMAGE3DOESPROC) SDL_GL_GetProcAddress("glTexImage3DOES")) != nullptr;
            v &= (glTexSubImage3DOES = (PFNGLTEXSUBIMAGE3DOESPROC) SDL_GL_GetProcAddress("glTexSubImage3DOES")) != nullptr;
            v &= (glCopyTexSubImage3DOES = (PFNGLCOPYTEXSUBIMAGE3DOESPROC) SDL_GL_GetProcAddress("glCopyTexSubImage3DOES")) != nullptr;
            v &= (glCompressedTexImage3DOES = (PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) SDL_GL_GetProcAddress("glCompressedTexImage3DOES")) != nullptr;
            v &= (glCompressedTexSubImage3DOES = (PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) SDL_GL_GetProcAddress("glCompressedTexSubImage3DOES")) != nullptr;
            glExt::OES_texture_3D = v;
        }
    #endif

    glExt::EXT_multisampled_render_to_texture = isExtensionSupported( "GL_EXT_multisampled_render_to_texture" );
    #ifdef NXGL_EXT_multisampled_render_to_texture
        if (glExt::EXT_multisampled_render_to_texture) {
            bool v = true;
            v &= (glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) SDL_GL_GetProcAddress("glFramebufferTexture2DMultisampleEXT")) != nullptr;
            v &= (glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) SDL_GL_GetProcAddress("glRenderbufferStorageMultisampleEXT")) != nullptr;
            glExt::EXT_multisampled_render_to_texture = v;
        }
    #endif

        // ANGLE_framebuffer_blit and ANGLE_framebuffer_multisample
    glExt::ANGLE_framebuffer_blit = glExt::ANGLE_framebuffer_multisample =
        isExtensionSupported("GL_ANGLE_framebuffer_multisample") && isExtensionSupported("GL_ANGLE_framebuffer_blit");

    #if defined(NXGL_ANGLE_framebuffer_blit) && defined(NXGL_ANGLE_framebuffer_multisample)
        if (glExt::ANGLE_framebuffer_multisample) {
            bool v = true;

            // From GL_ANGLE_framebuffer_blit
            v &= (glBlitFramebufferANGLE = (PFNGLBLITFRAMEBUFFERANGLEPROC) SDL_GL_GetProcAddress("glBlitFramebufferANGLE")) != nullptr;
            // From GL_ANGLE_framebuffer_multisample
            v &= (glRenderbufferStorageMultisampleANGLE = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC) SDL_GL_GetProcAddress("glRenderbufferStorageMultisampleANGLE")) != nullptr;

            glExt::ANGLE_framebuffer_blit = glExt::ANGLE_framebuffer_multisample = v;
        }
    #endif

    glExt::EXT_occlusion_query_boolean = isExtensionSupported("GL_EXT_occlusion_query_boolean");
    #ifdef NXGL_EXT_occlusion_query_boolean
        if (glExt::EXT_occlusion_query_boolean) {
            bool v = true;
            v &= (glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC) SDL_GL_GetProcAddress("glGenQueriesEXT")) != nullptr;
            v &= (glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC) SDL_GL_GetProcAddress("glDeleteQueriesEXT")) != nullptr;
            v &= (glIsQueryEXT = (PFNGLISQUERYEXTPROC) SDL_GL_GetProcAddress("glIsQueryEXT")) != nullptr;
            v &= (glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC) SDL_GL_GetProcAddress("glBeginQueryEXT")) != nullptr;
            v &= (glEndQueryEXT = (PFNGLENDQUERYEXTPROC) SDL_GL_GetProcAddress("glEndQueryEXT")) != nullptr;
            v &= (glGetQueryivEXT = (PFNGLGETQUERYIVEXTPROC) SDL_GL_GetProcAddress("glGetQueryivEXT")) != nullptr;
            v &= (glGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC) SDL_GL_GetProcAddress("glGetQueryObjectuivEXT")) != nullptr;
            glExt::EXT_occlusion_query_boolean = v;
        }
    #endif

    glExt::OES_rgb8_rgba8 = isExtensionSupported("GL_OES_rgb8_rgba8");

    glExt::EXT_texture_filter_anisotropic = isExtensionSupported("GL_EXT_texture_filter_anisotropic");

    glExt::IMG_texture_compression_pvrtc = isExtensionSupported("GL_IMG_texture_compression_pvrtc");

    glExt::EXT_texture_compression_s3tc = isExtensionSupported("GL_EXT_texture_compression_s3tc");
    glExt::EXT_texture_compression_dxt1 = isExtensionSupported("GL_EXT_texture_compression_dxt1");
    glExt::ANGLE_texture_compression_dxt3 = isExtensionSupported("GL_ANGLE_texture_compression_dxt3");
    glExt::ANGLE_texture_compression_dxt5 = isExtensionSupported("GL_ANGLE_texture_compression_dxt5");

    glExt::OES_compressed_ETC1_RGB8_texture = isExtensionSupported("GL_OES_compressed_ETC1_RGB8_texture");

    glExt::EXT_shadow_samplers = isExtensionSupported("GL_EXT_shadow_samplers");

    glExt::OES_depth_texture = isExtensionSupported("GL_OES_depth_texture");
    glExt::ANGLE_depth_texture = isExtensionSupported("GL_ANGLE_depth_texture");

    return true;
}

#endif
