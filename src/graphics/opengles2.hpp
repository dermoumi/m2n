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

#if defined(NX_OPENGL_ES)

// Check for OpenGL inclusion
#if defined(__glew_h__) || defined(__GLEW_H__)
    #error Attempt to include opengles2.hpp after including glew.h
#endif
#if defined(__gl_h_) || defined(__GL_H__)
    #error Attempt to include opengles2.hpp after including gl.h
#endif
#if defined(__gl2_h_) || defined(__GL2_H__)
    #error Attempt to include opengles2.hpp after including gl2.h
#endif
#if defined(__glext_h_) || defined(__GLEXT_H_)
    #error Attempt to include opengles2.hpp after including glext.h
#endif
#if defined(__gl2ext_h_) || defined(__GL2EXT_H_)
    #error Attempt to include opengles2.hpp after including gl2ext.h
#endif
#if defined(__gltypes_h_)
    #error Attempt to include opengles2.hpp after gltypes.h
#endif
#if defined(__gl_ATI_h_)
    #error Attempt to include opengles2.hpp after including glATI.h
#endif

// Define the APIENTRY macro
#ifndef APIENTRY
    #if defined(__MINGW32__)
        // Windows MinGW
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN 1
        #endif

        #ifndef NOMINMAX
            #define NOMINMAX
        #endif

        #include <windows.h>

    #elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED) || defined(__BORLANDC__)
        // Windows VC++
        #ifndef WIN32_LEAN_AND_MEAN
            #define WIN32_LEAN_AND_MEAN 1
        #endif

        #ifndef NOMINMAX
            #define NOMINMAX
        #endif

        #include <windows.h>

    #else
        // Everything else :/
        #define APIENTRY
    #endif
#endif

// Define the GLAPI macro
#ifndef GLAPI
    #define GLAPI extern
#endif

// Capabilities
namespace glExt
{
    extern bool EXT_multisampled_render_to_texture;

    extern bool ANGLE_framebuffer_blit;        // only true if ANGLE_framebuffer_multisample also supported
    extern bool ANGLE_framebuffer_multisample; // only true if ANGLE_framebuffer_blit also supported
    extern bool OES_rgb8_rgba8;

    extern bool EXT_texture_filter_anisotropic;
    extern bool EXT_disjoint_timer_query;
    extern bool EXT_occlusion_query_boolean; // supported on sgx 543+ and Angle (NaCL, emscripten)

    extern bool OES_texture_3D;

    extern bool EXT_texture_compression_s3tc;
    extern bool EXT_texture_compression_dxt1;
    extern bool ANGLE_texture_compression_dxt3;
    extern bool ANGLE_texture_compression_dxt5;

    extern bool IMG_texture_compression_pvrtc;
    extern bool OES_compressed_ETC1_RGB8_texture;

    extern bool OES_depth_texture;
    extern bool ANGLE_depth_texture;

    extern bool EXT_shadow_samplers;

    extern int  majorVersion, minorVersion;
}

// Initialization function
bool initOpenGLExtensions();

// Include GLES2 headers
#if defined(NX_SYSTEM_WINDOWS) || defined(NX_SYSTEM_WINCE)
    #include "GLES2/gl2.h"

#elif defined(NX_SYSTEM_IOS)
    #import <OpenGLES/ES2/gl.h>
    #import <OpenGLES/ES2/glext.h>
    #ifndef GL_GLEXT_PROTOTYPES
        #define GL_GLEXT_PROTOTYPES
    #endif

#elif defined(NX_SYSTEM_ANDROID)
    #ifndef GL_GLEXT_PROTOTYPES
        #define GL_GLEXT_PROTOTYPES
    #endif

    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#endif

// Defining missing defines
#if !defined(GL_MAJOR_VERSION)
    #define GL_MAJOR_VERSION 0x821B
#endif

#if !defined(GL_MINOR_VERSION)
    #define GL_MINOR_VERSION 0x821C
#endif

#if !defined(GL_MULTISAMPLE)
    #define GL_MULTISAMPLE 0x809D
#endif

// Extensions
// GL_OES_texture_3D
#ifndef GL_OES_texture_3D
    #define GL_OES_texture_3D 1
    #define NXGL_OES_texture_3D 1

    #define GL_TEXTURE_WRAP_R_OES                            0x8072
    #define GL_TEXTURE_3D_OES                                0x806F
    #define GL_TEXTURE_BINDING_3D_OES                        0x806A
    #define GL_MAX_3D_TEXTURE_SIZE_OES                       0x8073
    #define GL_SAMPLER_3D_OES                                0x8B5F
    #define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES 0x8CD4

    typedef void (APIENTRY* PFNGLTEXIMAGE3DOESPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
    typedef void (APIENTRY* PFNGLTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels);
    typedef void (APIENTRY* PFNGLCOPYTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    typedef void (APIENTRY* PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data);
    typedef void (APIENTRY* PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data);
    GLAPI PFNGLTEXIMAGE3DOESPROC glTexImage3DOES;
    GLAPI PFNGLTEXSUBIMAGE3DOESPROC glTexSubImage3DOES;
    GLAPI PFNGLCOPYTEXSUBIMAGE3DOESPROC glCopyTexSubImage3DOES;
    GLAPI PFNGLCOMPRESSEDTEXIMAGE3DOESPROC glCompressedTexImage3DOES;
    GLAPI PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC glCompressedTexSubImage3DOES;
#endif

// GL_EXT_shadow_samplers
#ifndef GL_EXT_shadow_samplers
    #define GL_TEXTURE_COMPARE_MODE_EXT                   0x884C
    #define GL_TEXTURE_COMPARE_FUNC_EXT                   0x884D
    #define GL_COMPARE_REF_TO_TEXTURE_EXT                 0x884E
    #define GL_SAMPLER_2D_SHADOW_EXT                      0x8B62
#endif

// GL_EXT_occlusion_query_boolean
#ifndef GL_EXT_occlusion_query_boolean
    #define GL_EXT_occlusion_query_boolean 1
    #define NXGL_EXT_occlusion_query_boolean 1

    #define GL_ANY_SAMPLES_PASSED_EXT                     0x8C2F
    #define GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT        0x8D6A
    #define GL_CURRENT_QUERY_EXT                          0x8865
    #define GL_QUERY_RESULT_EXT                           0x8866
    #define GL_QUERY_RESULT_AVAILABLE_EXT                 0x8867

    typedef void (APIENTRY* PFNGLGENQUERIESEXTPROC) (GLsizei n, GLuint *ids);
    typedef void (APIENTRY* PFNGLDELETEQUERIESEXTPROC) (GLsizei n, const GLuint *ids);
    typedef GLboolean (APIENTRY* PFNGLISQUERYEXTPROC) (GLuint id);
    typedef void (APIENTRY* PFNGLBEGINQUERYEXTPROC) (GLenum target, GLuint id);
    typedef void (APIENTRY* PFNGLENDQUERYEXTPROC) (GLenum target);
    typedef void (APIENTRY* PFNGLGETQUERYIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
    typedef void (APIENTRY* PFNGLGETQUERYOBJECTUIVEXTPROC) (GLuint id, GLenum pname, GLuint *params);

    GLAPI PFNGLGENQUERIESEXTPROC glGenQueriesEXT;
    GLAPI PFNGLDELETEQUERIESEXTPROC glDeleteQueriesEXT;
    GLAPI PFNGLISQUERYEXTPROC glIsQueryEXT;
    GLAPI PFNGLBEGINQUERYEXTPROC glBeginQueryEXT;
    GLAPI PFNGLENDQUERYEXTPROC glEndQueryEXT;
    GLAPI PFNGLGETQUERYIVEXTPROC glGetQueryivEXT;
    GLAPI PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuivEXT;
#endif

// EXT_texture_filter_anisotropic
#ifndef GL_EXT_texture_filter_anisotropic
    #define GL_EXT_texture_filter_anisotropic 1
    #define GL_TEXTURE_MAX_ANISOTROPY_EXT                 0x84FE
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT             0x84FF
#endif

// EXT_texture_compression_s3tc
#ifndef GL_EXT_texture_compression_s3tc
    #define GL_EXT_texture_compression_s3tc 1

    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT               0x83F0
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT              0x83F1
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT              0x83F2
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT              0x83F3
#endif

// GL_IMG_texture_compression_pvrtc
#ifndef GL_IMG_texture_compression_pvrtc
    #define GL_IMG_texture_compression_pvrtc 1
    #define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG            0x8C00
    #define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG            0x8C01
    #define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG           0x8C02
    #define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG           0x8C03
#endif

// GL_OES_compressed_ETC1
#ifndef GL_OES_compressed_ETC1_RGB8_texture
    #define GL_OES_compressed_ETC1_RGB8_texture
    #define GL_ETC1_RGB8_OES                              0x8D64
#endif

// GL_EXT_multisampled_render_to_texture
#ifndef GL_EXT_multisampled_render_to_texture
    #define GL_EXT_multisampled_render_to_texture 1
    #define NXGL_EXT_multisampled_render_to_texture 1

    #define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT 0x8D6C
    #define GL_RENDERBUFFER_SAMPLES_EXT                   0x9133
    #define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT     0x9134
    #define GL_MAX_SAMPLES_EXT                            0x9135

    typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);

    GLAPI PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
    GLAPI PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;
#endif

// GL_ANGLE_framebuffer_blit
#ifndef GL_ANGLE_framebuffer_blit
    #define GL_ANGLE_framebuffer_blit 1
    #define NXGL_ANGLE_framebuffer_blit 1

    #define GL_READ_FRAMEBUFFER_ANGLE                     0x8CA8
    #define GL_DRAW_FRAMEBUFFER_ANGLE                     0x8CA9
    #define GL_DRAW_FRAMEBUFFER_BINDING_ANGLE             0x8CA6
    #define GL_READ_FRAMEBUFFER_BINDING_ANGLE             0x8CAA

    typedef void (APIENTRY* PFNGLBLITFRAMEBUFFERANGLEPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

    GLAPI PFNGLBLITFRAMEBUFFERANGLEPROC glBlitFramebufferANGLE;
#endif

// GL_ANGLE_framebuffer_multisample
#ifndef GL_ANGLE_framebuffer_multisample
    #define GL_ANGLE_framebuffer_multisample 1
    #define NXGL_ANGLE_framebuffer_multisample 1

    #define GL_RENDERBUFFER_SAMPLES_ANGLE                 0x8CAB
    #define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE   0x8D56
    #define GL_MAX_SAMPLES_ANGLE                          0x8D57

    typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    GLAPI PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC glRenderbufferStorageMultisampleANGLE;
#endif

// GL_OES_rgb8_rgba8
#ifndef GL_OES_rgb8_rgba8
    #define GL_OES_rgb8_rgba8
    #define GL_RGB8_OES                                   0x8051
    #define GL_RGBA8_OES                                  0x8058
#endif

// EXT_disjoint_timer_query
#ifndef GL_EXT_disjoint_timer_query
    #define GL_EXT_disjoint_timer_query 1
    #define NXGL_EXT_disjoint_timer_query 1
#endif // GL_EXT_disjoint_timer_query

#endif
