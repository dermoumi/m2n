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

#if !defined(NX_OPENGL_ES)

// Check for OpenGL inclusion
#if defined(__glew_h__) || defined(__GLEW_H__)
    #error Attempt to include opengles2.hpp after including glew.h
#endif
#if defined(__gl_h_) || defined(__GL_H__)
    #error Attempt to include opengles2.hpp after including gl.h
#endif
#if defined(__glext_h_) || defined(__GLEXT_H_)
    #error Attempt to include opengles2.hpp after including glext.h
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

// Capabilities
namespace glExt
{
    extern bool EXT_framebuffer_object;
    extern bool EXT_framebuffer_multisample;
    extern bool EXT_texture_filter_anisotropic;
    extern bool EXT_texture_compression_s3tc;
    extern bool EXT_texture_sRGB;
    extern bool ARB_texture_float;
    extern bool ARB_texture_non_power_of_two;
    extern bool ARB_timer_query;

    extern int  majorVersion, minorVersion;
}

// Initialization function
bool initOpenGLExtensions();

// OpenGL 2.1 Functionality that has not been deprecated by GL3
// GL 1.1
#ifndef GL_VERSION_1_1
    #define GL_VERSION_1_1 1

    typedef unsigned int    GLenum;
    typedef unsigned char   GLboolean;
    typedef unsigned int    GLbitfield;
    typedef signed char     GLbyte;
    typedef short           GLshort;
    typedef int             GLint;
    typedef int             GLsizei;
    typedef unsigned char   GLubyte;
    typedef unsigned short  GLushort;
    typedef unsigned int    GLuint;
    typedef float           GLfloat;
    typedef float           GLclampf;
    typedef double          GLdouble;
    typedef double          GLclampd;
    typedef void            GLvoid;
    // ARB_timer_query
    typedef int64_t         GLint64;
    typedef uint64_t        GLuint64;

    #define GL_FALSE                          0
    #define GL_TRUE                           1

    #define GL_DEPTH_BUFFER_BIT               0x00000100
    #define GL_STENCIL_BUFFER_BIT             0x00000400
    #define GL_COLOR_BUFFER_BIT               0x00004000

    #define GL_POINTS                         0x0000
    #define GL_LINES                          0x0001
    #define GL_LINE_LOOP                      0x0002
    #define GL_LINE_STRIP                     0x0003
    #define GL_TRIANGLES                      0x0004
    #define GL_TRIANGLE_STRIP                 0x0005
    #define GL_TRIANGLE_FAN                   0x0006

    #define GL_NEVER                          0x0200
    #define GL_LESS                           0x0201
    #define GL_EQUAL                          0x0202
    #define GL_LEQUAL                         0x0203
    #define GL_GREATER                        0x0204
    #define GL_NOTEQUAL                       0x0205
    #define GL_GEQUAL                         0x0206
    #define GL_ALWAYS                         0x0207

    #define GL_ZERO                           0
    #define GL_ONE                            1
    #define GL_SRC_COLOR                      0x0300
    #define GL_ONE_MINUS_SRC_COLOR            0x0301
    #define GL_SRC_ALPHA                      0x0302
    #define GL_ONE_MINUS_SRC_ALPHA            0x0303
    #define GL_DST_ALPHA                      0x0304
    #define GL_ONE_MINUS_DST_ALPHA            0x0305
    #define GL_DST_COLOR                      0x0306
    #define GL_ONE_MINUS_DST_COLOR            0x0307
    #define GL_SRC_ALPHA_SATURATE             0x0308

    #define GL_NONE                           0
    #define GL_FRONT_LEFT                     0x0400
    #define GL_FRONT_RIGHT                    0x0401
    #define GL_BACK_LEFT                      0x0402
    #define GL_BACK_RIGHT                     0x0403
    #define GL_FRONT                          0x0404
    #define GL_BACK                           0x0405
    #define GL_LEFT                           0x0406
    #define GL_RIGHT                          0x0407
    #define GL_FRONT_AND_BACK                 0x0408

    #define GL_NO_ERROR                       0
    #define GL_INVALID_ENUM                   0x0500
    #define GL_INVALID_VALUE                  0x0501
    #define GL_INVALID_OPERATION              0x0502
    #define GL_STACK_OVERFLOW                 0x0503
    #define GL_STACK_UNDERFLOW                0x0504
    #define GL_OUT_OF_MEMORY                  0x0505

    #define GL_CW                             0x0900
    #define GL_CCW                            0x0901

    #define GL_POINT_SIZE                     0x0B11
    #define GL_POINT_SIZE_RANGE               0x0B12
    #define GL_POINT_SIZE_GRANULARITY         0x0B13
    #define GL_LINE_SMOOTH                    0x0B20
    #define GL_LINE_WIDTH                     0x0B21
    #define GL_LINE_WIDTH_RANGE               0x0B22
    #define GL_LINE_WIDTH_GRANULARITY         0x0B23
    #define GL_POLYGON_MODE                   0x0B40
    #define GL_POLYGON_SMOOTH                 0x0B41
    #define GL_CULL_FACE                      0x0B44
    #define GL_CULL_FACE_MODE                 0x0B45
    #define GL_FRONT_FACE                     0x0B46
    #define GL_DEPTH_RANGE                    0x0B70
    #define GL_DEPTH_TEST                     0x0B71
    #define GL_DEPTH_WRITEMASK                0x0B72
    #define GL_DEPTH_CLEAR_VALUE              0x0B73
    #define GL_DEPTH_FUNC                     0x0B74
    #define GL_STENCIL_TEST                   0x0B90
    #define GL_STENCIL_CLEAR_VALUE            0x0B91
    #define GL_STENCIL_FUNC                   0x0B92
    #define GL_STENCIL_VALUE_MASK             0x0B93
    #define GL_STENCIL_FAIL                   0x0B94
    #define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
    #define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
    #define GL_STENCIL_REF                    0x0B97
    #define GL_STENCIL_WRITEMASK              0x0B98
    #define GL_VIEWPORT                       0x0BA2
    #define GL_DITHER                         0x0BD0
    #define GL_BLEND_DST                      0x0BE0
    #define GL_BLEND_SRC                      0x0BE1
    #define GL_BLEND                          0x0BE2
    #define GL_LOGIC_OP_MODE                  0x0BF0
    #define GL_COLOR_LOGIC_OP                 0x0BF2
    #define GL_DRAW_BUFFER                    0x0C01
    #define GL_READ_BUFFER                    0x0C02
    #define GL_SCISSOR_BOX                    0x0C10
    #define GL_SCISSOR_TEST                   0x0C11
    #define GL_COLOR_CLEAR_VALUE              0x0C22
    #define GL_COLOR_WRITEMASK                0x0C23
    #define GL_RGBA_MODE                      0x0C31
    #define GL_DOUBLEBUFFER                   0x0C32
    #define GL_STEREO                         0x0C33
    #define GL_RENDER_MODE                    0x0C40
    #define GL_LINE_SMOOTH_HINT               0x0C52
    #define GL_POLYGON_SMOOTH_HINT            0x0C53
    #define GL_UNPACK_SWAP_BYTES              0x0CF0
    #define GL_UNPACK_LSB_FIRST               0x0CF1
    #define GL_UNPACK_ROW_LENGTH              0x0CF2
    #define GL_UNPACK_SKIP_ROWS               0x0CF3
    #define GL_UNPACK_SKIP_PIXELS             0x0CF4
    #define GL_UNPACK_ALIGNMENT               0x0CF5
    #define GL_PACK_SWAP_BYTES                0x0D00
    #define GL_PACK_LSB_FIRST                 0x0D01
    #define GL_PACK_ROW_LENGTH                0x0D02
    #define GL_PACK_SKIP_ROWS                 0x0D03
    #define GL_PACK_SKIP_PIXELS               0x0D04
    #define GL_PACK_ALIGNMENT                 0x0D05
    #define GL_MAX_TEXTURE_SIZE               0x0D33
    #define GL_MAX_VIEWPORT_DIMS              0x0D3A
    #define GL_SUBPIXEL_BITS                  0x0D50
    #define GL_TEXTURE_1D                     0x0DE0
    #define GL_TEXTURE_2D                     0x0DE1
    #define GL_POLYGON_OFFSET_UNITS           0x2A00
    #define GL_POLYGON_OFFSET_POINT           0x2A01
    #define GL_POLYGON_OFFSET_LINE            0x2A02
    #define GL_POLYGON_OFFSET_FILL            0x8037
    #define GL_POLYGON_OFFSET_FACTOR          0x8038
    #define GL_TEXTURE_BINDING_1D             0x8068
    #define GL_TEXTURE_BINDING_2D             0x8069

    #define GL_TEXTURE_WIDTH                  0x1000
    #define GL_TEXTURE_HEIGHT                 0x1001
    #define GL_TEXTURE_INTERNAL_FORMAT        0x1003
    #define GL_TEXTURE_BORDER_COLOR           0x1004
    #define GL_TEXTURE_BORDER                 0x1005
    #define GL_TEXTURE_RED_SIZE               0x805C
    #define GL_TEXTURE_GREEN_SIZE             0x805D
    #define GL_TEXTURE_BLUE_SIZE              0x805E
    #define GL_TEXTURE_ALPHA_SIZE             0x805F

    #define GL_DONT_CARE                      0x1100
    #define GL_FASTEST                        0x1101
    #define GL_NICEST                         0x1102

    #define GL_BYTE                           0x1400
    #define GL_UNSIGNED_BYTE                  0x1401
    #define GL_SHORT                          0x1402
    #define GL_UNSIGNED_SHORT                 0x1403
    #define GL_INT                            0x1404
    #define GL_UNSIGNED_INT                   0x1405
    #define GL_FLOAT                          0x1406
    #define GL_DOUBLE                         0x140A

    #define GL_CLEAR                          0x1500
    #define GL_AND                            0x1501
    #define GL_AND_REVERSE                    0x1502
    #define GL_COPY                           0x1503
    #define GL_AND_INVERTED                   0x1504
    #define GL_NOOP                           0x1505
    #define GL_XOR                            0x1506
    #define GL_OR                             0x1507
    #define GL_NOR                            0x1508
    #define GL_EQUIV                          0x1509
    #define GL_INVERT                         0x150A
    #define GL_OR_REVERSE                     0x150B
    #define GL_COPY_INVERTED                  0x150C
    #define GL_OR_INVERTED                    0x150D
    #define GL_NAND                           0x150E
    #define GL_SET                            0x150F

    #define GL_STENCIL_INDEX                  0x1901
    #define GL_DEPTH_COMPONENT                0x1902
    #define GL_RED                            0x1903
    #define GL_GREEN                          0x1904
    #define GL_BLUE                           0x1905
    #define GL_ALPHA                          0x1906
    #define GL_RGB                            0x1907
    #define GL_RGBA                           0x1908

    #define GL_POINT                          0x1B00
    #define GL_LINE                           0x1B01
    #define GL_FILL                           0x1B02

    #define GL_KEEP                           0x1E00
    #define GL_REPLACE                        0x1E01
    #define GL_INCR                           0x1E02
    #define GL_DECR                           0x1E03

    #define GL_VENDOR                         0x1F00
    #define GL_RENDERER                       0x1F01
    #define GL_VERSION                        0x1F02
    #define GL_EXTENSIONS                     0x1F03

    #define GL_NEAREST                        0x2600
    #define GL_LINEAR                         0x2601

    #define GL_NEAREST_MIPMAP_NEAREST         0x2700
    #define GL_LINEAR_MIPMAP_NEAREST          0x2701
    #define GL_NEAREST_MIPMAP_LINEAR          0x2702
    #define GL_LINEAR_MIPMAP_LINEAR           0x2703

    #define GL_TEXTURE_MAG_FILTER             0x2800
    #define GL_TEXTURE_MIN_FILTER             0x2801
    #define GL_TEXTURE_WRAP_S                 0x2802
    #define GL_TEXTURE_WRAP_T                 0x2803

    #define GL_REPEAT                         0x2901

    #define GL_R3_G3_B2                       0x2A10
    #define GL_RGB4                           0x804F
    #define GL_RGB5                           0x8050
    #define GL_RGB8                           0x8051
    #define GL_RGB10                          0x8052
    #define GL_RGB12                          0x8053
    #define GL_RGB16                          0x8054
    #define GL_RGBA2                          0x8055
    #define GL_RGBA4                          0x8056
    #define GL_RGB5_A1                        0x8057
    #define GL_RGBA8                          0x8058
    #define GL_RGB10_A2                       0x8059
    #define GL_RGBA12                         0x805A
    #define GL_RGBA16                         0x805B

    #define GL_PROXY_TEXTURE_1D               0x8063
    #define GL_PROXY_TEXTURE_2D               0x8064

    extern "C"
    {
        GLAPI void APIENTRY glBindTexture (GLenum target, GLuint texture);
        GLAPI void APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor);
        GLAPI void APIENTRY glClear (GLbitfield mask);
        GLAPI void APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
        GLAPI void APIENTRY glClearDepth (GLclampd depth);
        GLAPI void APIENTRY glClearStencil (GLint s);
        GLAPI void APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
        GLAPI void APIENTRY glCopyTexImage1D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border);
        GLAPI void APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
        GLAPI void APIENTRY glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
        GLAPI void APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
        GLAPI void APIENTRY glCullFace (GLenum mode);
        GLAPI void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures);
        GLAPI void APIENTRY glDepthFunc (GLenum func);
        GLAPI void APIENTRY glDepthMask (GLboolean flag);
        GLAPI void APIENTRY glDepthRange (GLclampd zNear, GLclampd zFar);
        GLAPI void APIENTRY glDisable (GLenum cap);
        GLAPI void APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count);
        GLAPI void APIENTRY glDrawBuffer (GLenum mode);
        GLAPI void APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
        GLAPI void APIENTRY glEnable (GLenum cap);
        GLAPI void APIENTRY glFinish (void);
        GLAPI void APIENTRY glFlush (void);
        GLAPI void APIENTRY glFrontFace (GLenum mode);
        GLAPI void APIENTRY glGenTextures (GLsizei n, GLuint *textures);
        GLAPI void APIENTRY glGetBooleanv (GLenum pname, GLboolean *params);
        GLAPI void APIENTRY glGetDoublev (GLenum pname, GLdouble *params);
        GLAPI GLenum APIENTRY glGetError (void);
        GLAPI void APIENTRY glGetFloatv (GLenum pname, GLfloat *params);
        GLAPI void APIENTRY glGetIntegerv (GLenum pname, GLint *params);
        GLAPI void APIENTRY glGetPointerv (GLenum pname, GLvoid* *params);
        GLAPI const GLubyte * APIENTRY glGetString (GLenum name);
        GLAPI void APIENTRY glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
        GLAPI void APIENTRY glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params);
        GLAPI void APIENTRY glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params);
        GLAPI void APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params);
        GLAPI void APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params);
        GLAPI void APIENTRY glHint (GLenum target, GLenum mode);
        GLAPI GLboolean APIENTRY glIsEnabled (GLenum cap);
        GLAPI GLboolean APIENTRY glIsTexture (GLuint texture);
        GLAPI void APIENTRY glLineWidth (GLfloat width);
        GLAPI void APIENTRY glLogicOp (GLenum opcode);
        GLAPI void APIENTRY glPixelStoref (GLenum pname, GLfloat param);
        GLAPI void APIENTRY glPixelStorei (GLenum pname, GLint param);
        GLAPI void APIENTRY glPointSize (GLfloat size);
        GLAPI void APIENTRY glPolygonMode (GLenum face, GLenum mode);
        GLAPI void APIENTRY glPolygonOffset (GLfloat factor, GLfloat units);
        GLAPI void APIENTRY glReadBuffer (GLenum mode);
        GLAPI void APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
        GLAPI void APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height);
        GLAPI void APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask);
        GLAPI void APIENTRY glStencilMask (GLuint mask);
        GLAPI void APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass);
        GLAPI void APIENTRY glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
        GLAPI void APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
        GLAPI void APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param);
        GLAPI void APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params);
        GLAPI void APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param);
        GLAPI void APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params);
        GLAPI void APIENTRY glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
        GLAPI void APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
        GLAPI void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height);
    }  // extern "C"
#endif  // GL_VERSION_1_1

// GL 1.2
#ifndef GL_VERSION_1_2
    #define GL_VERSION_1_2 1

    #define GL_UNSIGNED_BYTE_3_3_2            0x8032
    #define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
    #define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
    #define GL_UNSIGNED_INT_8_8_8_8           0x8035
    #define GL_UNSIGNED_INT_10_10_10_2        0x8036
    #define GL_TEXTURE_BINDING_3D             0x806A
    #define GL_PACK_SKIP_IMAGES               0x806B
    #define GL_PACK_IMAGE_HEIGHT              0x806C
    #define GL_UNPACK_SKIP_IMAGES             0x806D
    #define GL_UNPACK_IMAGE_HEIGHT            0x806E
    #define GL_TEXTURE_3D                     0x806F
    #define GL_PROXY_TEXTURE_3D               0x8070
    #define GL_TEXTURE_DEPTH                  0x8071
    #define GL_TEXTURE_WRAP_R                 0x8072
    #define GL_MAX_3D_TEXTURE_SIZE            0x8073
    #define GL_UNSIGNED_BYTE_2_3_3_REV        0x8362
    #define GL_UNSIGNED_SHORT_5_6_5           0x8363
    #define GL_UNSIGNED_SHORT_5_6_5_REV       0x8364
    #define GL_UNSIGNED_SHORT_4_4_4_4_REV     0x8365
    #define GL_UNSIGNED_SHORT_1_5_5_5_REV     0x8366
    #define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367
    #define GL_UNSIGNED_INT_2_10_10_10_REV    0x8368
    #define GL_BGR                            0x80E0
    #define GL_BGRA                           0x80E1
    #define GL_MAX_ELEMENTS_VERTICES          0x80E8
    #define GL_MAX_ELEMENTS_INDICES           0x80E9
    #define GL_CLAMP_TO_EDGE                  0x812F
    #define GL_TEXTURE_MIN_LOD                0x813A
    #define GL_TEXTURE_MAX_LOD                0x813B
    #define GL_TEXTURE_BASE_LEVEL             0x813C
    #define GL_TEXTURE_MAX_LEVEL              0x813D
    #define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
    #define GL_SMOOTH_POINT_SIZE_GRANULARITY  0x0B13
    #define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
    #define GL_SMOOTH_LINE_WIDTH_GRANULARITY  0x0B23
    #define GL_ALIASED_POINT_SIZE_RANGE       0x846D
    #define GL_ALIASED_LINE_WIDTH_RANGE       0x846E

    typedef void (APIENTRY* PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    typedef void (APIENTRY* PFNGLBLENDEQUATIONPROC) (GLenum mode);
    typedef void (APIENTRY* PFNGLDRAWRANGEELEMENTSPROC) (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    typedef void (APIENTRY* PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    typedef void (APIENTRY* PFNGLTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
    typedef void (APIENTRY* PFNGLCOPYTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

    GLAPI PFNGLBLENDCOLORPROC glBlendColor;
    GLAPI PFNGLBLENDEQUATIONPROC glBlendEquation;
    GLAPI PFNGLDRAWRANGEELEMENTSPROC glDrawRangeElements;
    GLAPI PFNGLTEXIMAGE3DPROC glTexImage3D;
    GLAPI PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;
    GLAPI PFNGLCOPYTEXSUBIMAGE3DPROC glCopyTexSubImage3D;
#endif  // GL_VERSION_1_2

// GL 1.3
#ifndef GL_VERSION_1_3
    #define GL_VERSION_1_3 1

    #define GL_TEXTURE0                       0x84C0
    #define GL_TEXTURE1                       0x84C1
    #define GL_TEXTURE2                       0x84C2
    #define GL_TEXTURE3                       0x84C3
    #define GL_TEXTURE4                       0x84C4
    #define GL_TEXTURE5                       0x84C5
    #define GL_TEXTURE6                       0x84C6
    #define GL_TEXTURE7                       0x84C7
    #define GL_TEXTURE8                       0x84C8
    #define GL_TEXTURE9                       0x84C9
    #define GL_TEXTURE10                      0x84CA
    #define GL_TEXTURE11                      0x84CB
    #define GL_TEXTURE12                      0x84CC
    #define GL_TEXTURE13                      0x84CD
    #define GL_TEXTURE14                      0x84CE
    #define GL_TEXTURE15                      0x84CF
    #define GL_TEXTURE16                      0x84D0
    #define GL_TEXTURE17                      0x84D1
    #define GL_TEXTURE18                      0x84D2
    #define GL_TEXTURE19                      0x84D3
    #define GL_TEXTURE20                      0x84D4
    #define GL_TEXTURE21                      0x84D5
    #define GL_TEXTURE22                      0x84D6
    #define GL_TEXTURE23                      0x84D7
    #define GL_TEXTURE24                      0x84D8
    #define GL_TEXTURE25                      0x84D9
    #define GL_TEXTURE26                      0x84DA
    #define GL_TEXTURE27                      0x84DB
    #define GL_TEXTURE28                      0x84DC
    #define GL_TEXTURE29                      0x84DD
    #define GL_TEXTURE30                      0x84DE
    #define GL_TEXTURE31                      0x84DF
    #define GL_ACTIVE_TEXTURE                 0x84E0
    #define GL_MULTISAMPLE                    0x809D
    #define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
    #define GL_SAMPLE_ALPHA_TO_ONE            0x809F
    #define GL_SAMPLE_COVERAGE                0x80A0
    #define GL_SAMPLE_BUFFERS                 0x80A8
    #define GL_SAMPLES                        0x80A9
    #define GL_SAMPLE_COVERAGE_VALUE          0x80AA
    #define GL_SAMPLE_COVERAGE_INVERT         0x80AB
    #define GL_TEXTURE_CUBE_MAP               0x8513
    #define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
    #define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
    #define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
    #define GL_PROXY_TEXTURE_CUBE_MAP         0x851B
    #define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C
    #define GL_COMPRESSED_RGB                 0x84ED
    #define GL_COMPRESSED_RGBA                0x84EE
    #define GL_TEXTURE_COMPRESSION_HINT       0x84EF
    #define GL_TEXTURE_COMPRESSED_IMAGE_SIZE  0x86A0
    #define GL_TEXTURE_COMPRESSED             0x86A1
    #define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
    #define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3
    #define GL_CLAMP_TO_BORDER                0x812D

    typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC) (GLenum texture);
    typedef void (APIENTRY* PFNGLSAMPLECOVERAGEPROC) (GLclampf value, GLboolean invert);
    typedef void (APIENTRY* PFNGLCOMPRESSEDTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
    typedef void (APIENTRY* PFNGLCOMPRESSEDTEXIMAGE2DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
    typedef void (APIENTRY* PFNGLCOMPRESSEDTEXIMAGE1DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
    typedef void (APIENTRY* PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
    typedef void (APIENTRY* PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
    typedef void (APIENTRY* PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC) (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
    typedef void (APIENTRY* PFNGLGETCOMPRESSEDTEXIMAGEPROC) (GLenum target, GLint level, GLvoid *img);

    GLAPI PFNGLACTIVETEXTUREPROC glActiveTexture;
    GLAPI PFNGLSAMPLECOVERAGEPROC glSampleCoverage;
    GLAPI PFNGLCOMPRESSEDTEXIMAGE3DPROC glCompressedTexImage3D;
    GLAPI PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
    GLAPI PFNGLCOMPRESSEDTEXIMAGE1DPROC glCompressedTexImage1D;
    GLAPI PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC glCompressedTexSubImage3D;
    GLAPI PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D;
    GLAPI PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC glCompressedTexSubImage1D;
    GLAPI PFNGLGETCOMPRESSEDTEXIMAGEPROC glGetCompressedTexImage;
#endif // GL 1.3

// GL 1.4
#ifndef GL_VERSION_1_4
    #define GL_VERSION_1_4 1

    #define GL_BLEND_DST_RGB                  0x80C8
    #define GL_BLEND_SRC_RGB                  0x80C9
    #define GL_BLEND_DST_ALPHA                0x80CA
    #define GL_BLEND_SRC_ALPHA                0x80CB
    #define GL_POINT_SIZE_MIN                 0x8126
    #define GL_POINT_SIZE_MAX                 0x8127
    #define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
    #define GL_POINT_DISTANCE_ATTENUATION     0x8129
    #define GL_DEPTH_COMPONENT16              0x81A5
    #define GL_DEPTH_COMPONENT24              0x81A6
    #define GL_DEPTH_COMPONENT32              0x81A7
    #define GL_MIRRORED_REPEAT                0x8370
    #define GL_INCR_WRAP                      0x8507
    #define GL_DECR_WRAP                      0x8508
    #define GL_TEXTURE_DEPTH_SIZE             0x884A
    #define GL_DEPTH_TEXTURE_MODE             0x884B
    #define GL_TEXTURE_COMPARE_MODE           0x884C
    #define GL_TEXTURE_COMPARE_FUNC           0x884D
    #define GL_COMPARE_R_TO_TEXTURE           0x884E

    typedef void (APIENTRY* PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
    typedef void (APIENTRY* PFNGLMULTIDRAWARRAYSPROC) (GLenum mode, GLint *first, GLsizei *count, GLsizei primcount);
    typedef void (APIENTRY* PFNGLMULTIDRAWELEMENTSPROC) (GLenum mode, const GLsizei *count, GLenum type, const GLvoid* *indices, GLsizei primcount);
    typedef void (APIENTRY* PFNGLPOINTPARAMETERFPROC) (GLenum pname, GLfloat param);
    typedef void (APIENTRY* PFNGLPOINTPARAMETERFVPROC) (GLenum pname, const GLfloat *params);
    typedef void (APIENTRY* PFNGLPOINTPARAMETERIPROC) (GLenum pname, GLint param);
    typedef void (APIENTRY* PFNGLPOINTPARAMETERIVPROC) (GLenum pname, const GLint *params);

    GLAPI PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
    GLAPI PFNGLMULTIDRAWARRAYSPROC glMultiDrawArrays;
    GLAPI PFNGLMULTIDRAWELEMENTSPROC glMultiDrawElements;
    GLAPI PFNGLPOINTPARAMETERFPROC glPointParameterf;
    GLAPI PFNGLPOINTPARAMETERFVPROC glPointParameterfv;
    GLAPI PFNGLPOINTPARAMETERIPROC glPointParameteri;
    GLAPI PFNGLPOINTPARAMETERIVPROC glPointParameteriv;
#endif  // GL_VERSION_1_4

// GL 1.5
#ifndef GL_VERSION_1_5
    #define GL_VERSION_1_5 1

    typedef ptrdiff_t   GLintptr;
    typedef ptrdiff_t   GLsizeiptr;

    #define GL_BUFFER_SIZE                          0x8764
    #define GL_BUFFER_USAGE                         0x8765
    #define GL_QUERY_COUNTER_BITS                   0x8864
    #define GL_CURRENT_QUERY                        0x8865
    #define GL_QUERY_RESULT                         0x8866
    #define GL_QUERY_RESULT_AVAILABLE               0x8867
    #define GL_ARRAY_BUFFER                         0x8892
    #define GL_ELEMENT_ARRAY_BUFFER                 0x8893
    #define GL_ARRAY_BUFFER_BINDING                 0x8894
    #define GL_ELEMENT_ARRAY_BUFFER_BINDING         0x8895
    #define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING   0x889F
    #define GL_READ_ONLY                            0x88B8
    #define GL_WRITE_ONLY                           0x88B9
    #define GL_READ_WRITE                           0x88BA
    #define GL_BUFFER_ACCESS                        0x88BB
    #define GL_BUFFER_MAPPED                        0x88BC
    #define GL_BUFFER_MAP_POINTER                   0x88BD
    #define GL_STREAM_DRAW                          0x88E0
    #define GL_STREAM_READ                          0x88E1
    #define GL_STREAM_COPY                          0x88E2
    #define GL_STATIC_DRAW                          0x88E4
    #define GL_STATIC_READ                          0x88E5
    #define GL_STATIC_COPY                          0x88E6
    #define GL_DYNAMIC_DRAW                         0x88E8
    #define GL_DYNAMIC_READ                         0x88E9
    #define GL_DYNAMIC_COPY                         0x88EA
    #define GL_SAMPLES_PASSED                       0x8914
    #define GL_SRC0_RGB                             GL_SOURCE0_RGB
    #define GL_SRC1_RGB                             GL_SOURCE1_RGB
    #define GL_SRC2_RGB                             GL_SOURCE2_RGB
    #define GL_SRC0_ALPHA                           GL_SOURCE0_ALPHA
    #define GL_SRC1_ALPHA                           GL_SOURCE1_ALPHA
    #define GL_SRC2_ALPHA                           GL_SOURCE2_ALPHA

    typedef void (APIENTRY* PFNGLGENQUERIESPROC) (GLsizei n, GLuint *ids);
    typedef void (APIENTRY* PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint *ids);
    typedef GLboolean (APIENTRY* PFNGLISQUERYPROC) (GLuint id);
    typedef void (APIENTRY* PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
    typedef void (APIENTRY* PFNGLENDQUERYPROC) (GLenum target);
    typedef void (APIENTRY* PFNGLGETQUERYIVPROC) (GLenum target, GLenum pname, GLint *params);
    typedef void (APIENTRY* PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint *params);
    typedef void (APIENTRY* PFNGLGETQUERYOBJECTUIVPROC) (GLuint id, GLenum pname, GLuint *params);
    typedef void (APIENTRY* PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
    typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
    typedef void (APIENTRY* PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
    typedef GLboolean (APIENTRY* PFNGLISBUFFERPROC) (GLuint buffer);
    typedef void (APIENTRY* PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
    typedef void (APIENTRY* PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
    typedef void (APIENTRY* PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
    typedef GLvoid* (APIENTRY* PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
    typedef GLboolean (APIENTRY* PFNGLUNMAPBUFFERPROC) (GLenum target);
    typedef void (APIENTRY* PFNGLGETBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint *params);
    typedef void (APIENTRY* PFNGLGETBUFFERPOINTERVPROC) (GLenum target, GLenum pname, GLvoid* *params);

    GLAPI PFNGLGENQUERIESPROC glGenQueries;
    GLAPI PFNGLDELETEQUERIESPROC glDeleteQueries;
    GLAPI PFNGLISQUERYPROC glIsQuery;
    GLAPI PFNGLBEGINQUERYPROC glBeginQuery;
    GLAPI PFNGLENDQUERYPROC glEndQuery;
    GLAPI PFNGLGETQUERYIVPROC glGetQueryiv;
    GLAPI PFNGLGETQUERYOBJECTIVPROC glGetQueryObjectiv;
    GLAPI PFNGLGETQUERYOBJECTUIVPROC glGetQueryObjectuiv;
    GLAPI PFNGLBINDBUFFERPROC glBindBuffer;
    GLAPI PFNGLDELETEBUFFERSPROC glDeleteBuffers;
    GLAPI PFNGLGENBUFFERSPROC glGenBuffers;
    GLAPI PFNGLISBUFFERPROC glIsBuffer;
    GLAPI PFNGLBUFFERDATAPROC glBufferData;
    GLAPI PFNGLBUFFERSUBDATAPROC glBufferSubData;
    GLAPI PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;
    GLAPI PFNGLMAPBUFFERPROC glMapBuffer;
    GLAPI PFNGLUNMAPBUFFERPROC glUnmapBuffer;
    GLAPI PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;
    GLAPI PFNGLGETBUFFERPOINTERVPROC glGetBufferPointerv;
#endif  // GL_VERSION_1_5

// GL 2.0
#ifndef GL_VERSION_2_0
    #define GL_VERSION_2_0 1

    typedef char    GLchar;

    #define GL_BLEND_EQUATION_RGB               GL_BLEND_EQUATION
    #define GL_VERTEX_ATTRIB_ARRAY_ENABLED      0x8622
    #define GL_VERTEX_ATTRIB_ARRAY_SIZE         0x8623
    #define GL_VERTEX_ATTRIB_ARRAY_STRIDE       0x8624
    #define GL_VERTEX_ATTRIB_ARRAY_TYPE         0x8625
    #define GL_CURRENT_VERTEX_ATTRIB            0x8626
    #define GL_VERTEX_PROGRAM_POINT_SIZE        0x8642
    #define GL_VERTEX_ATTRIB_ARRAY_POINTER      0x8645
    #define GL_STENCIL_BACK_FUNC                0x8800
    #define GL_STENCIL_BACK_FAIL                0x8801
    #define GL_STENCIL_BACK_PASS_DEPTH_FAIL     0x8802
    #define GL_STENCIL_BACK_PASS_DEPTH_PASS     0x8803
    #define GL_MAX_DRAW_BUFFERS                 0x8824
    #define GL_DRAW_BUFFER0                     0x8825
    #define GL_DRAW_BUFFER1                     0x8826
    #define GL_DRAW_BUFFER2                     0x8827
    #define GL_DRAW_BUFFER3                     0x8828
    #define GL_DRAW_BUFFER4                     0x8829
    #define GL_DRAW_BUFFER5                     0x882A
    #define GL_DRAW_BUFFER6                     0x882B
    #define GL_DRAW_BUFFER7                     0x882C
    #define GL_DRAW_BUFFER8                     0x882D
    #define GL_DRAW_BUFFER9                     0x882E
    #define GL_DRAW_BUFFER10                    0x882F
    #define GL_DRAW_BUFFER11                    0x8830
    #define GL_DRAW_BUFFER12                    0x8831
    #define GL_DRAW_BUFFER13                    0x8832
    #define GL_DRAW_BUFFER14                    0x8833
    #define GL_DRAW_BUFFER15                    0x8834
    #define GL_BLEND_EQUATION_ALPHA             0x883D
    #define GL_POINT_SPRITE                     0x8861
    #define GL_COORD_REPLACE                    0x8862
    #define GL_MAX_VERTEX_ATTRIBS               0x8869
    #define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED   0x886A
    #define GL_MAX_TEXTURE_IMAGE_UNITS          0x8872
    #define GL_FRAGMENT_SHADER                  0x8B30
    #define GL_VERTEX_SHADER                    0x8B31
    #define GL_MAX_FRAGMENT_UNIFORM_COMPONENTS  0x8B49
    #define GL_MAX_VERTEX_UNIFORM_COMPONENTS    0x8B4A
    #define GL_MAX_VARYING_FLOATS               0x8B4B
    #define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS   0x8B4C
    #define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
    #define GL_SHADER_TYPE                      0x8B4F
    #define GL_FLOAT_VEC2                       0x8B50
    #define GL_FLOAT_VEC3                       0x8B51
    #define GL_FLOAT_VEC4                       0x8B52
    #define GL_INT_VEC2                         0x8B53
    #define GL_INT_VEC3                         0x8B54
    #define GL_INT_VEC4                         0x8B55
    #define GL_BOOL                             0x8B56
    #define GL_BOOL_VEC2                        0x8B57
    #define GL_BOOL_VEC3                        0x8B58
    #define GL_BOOL_VEC4                        0x8B59
    #define GL_FLOAT_MAT2                       0x8B5A
    #define GL_FLOAT_MAT3                       0x8B5B
    #define GL_FLOAT_MAT4                       0x8B5C
    #define GL_SAMPLER_1D                       0x8B5D
    #define GL_SAMPLER_2D                       0x8B5E
    #define GL_SAMPLER_3D                       0x8B5F
    #define GL_SAMPLER_CUBE                     0x8B60
    #define GL_SAMPLER_1D_SHADOW                0x8B61
    #define GL_SAMPLER_2D_SHADOW                0x8B62
    #define GL_DELETE_STATUS                    0x8B80
    #define GL_COMPILE_STATUS                   0x8B81
    #define GL_LINK_STATUS                      0x8B82
    #define GL_VALIDATE_STATUS                  0x8B83
    #define GL_INFO_LOG_LENGTH                  0x8B84
    #define GL_ATTACHED_SHADERS                 0x8B85
    #define GL_ACTIVE_UNIFORMS                  0x8B86
    #define GL_ACTIVE_UNIFORM_MAX_LENGTH        0x8B87
    #define GL_SHADER_SOURCE_LENGTH             0x8B88
    #define GL_ACTIVE_ATTRIBUTES                0x8B89
    #define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH      0x8B8A
    #define GL_FRAGMENT_SHADER_DERIVATIVE_HINT  0x8B8B
    #define GL_SHADING_LANGUAGE_VERSION         0x8B8C
    #define GL_CURRENT_PROGRAM                  0x8B8D
    #define GL_POINT_SPRITE_COORD_ORIGIN        0x8CA0
    #define GL_LOWER_LEFT                       0x8CA1
    #define GL_UPPER_LEFT                       0x8CA2
    #define GL_STENCIL_BACK_REF                 0x8CA3
    #define GL_STENCIL_BACK_VALUE_MASK          0x8CA4
    #define GL_STENCIL_BACK_WRITEMASK           0x8CA5

    typedef void (APIENTRY* PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum modeRGB, GLenum modeAlpha);
    typedef void (APIENTRY* PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
    typedef void (APIENTRY* PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    typedef void (APIENTRY* PFNGLSTENCILFUNCSEPARATEPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
    typedef void (APIENTRY* PFNGLSTENCILMASKSEPARATEPROC) (GLenum face, GLuint mask);
    typedef void (APIENTRY* PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
    typedef void (APIENTRY* PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
    typedef void (APIENTRY* PFNGLCOMPILESHADERPROC) (GLuint shader);
    typedef GLuint (APIENTRY* PFNGLCREATEPROGRAMPROC) (void);
    typedef GLuint (APIENTRY* PFNGLCREATESHADERPROC) (GLenum type);
    typedef void (APIENTRY* PFNGLDELETEPROGRAMPROC) (GLuint program);
    typedef void (APIENTRY* PFNGLDELETESHADERPROC) (GLuint shader);
    typedef void (APIENTRY* PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
    typedef void (APIENTRY* PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint index);
    typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
    typedef void (APIENTRY* PFNGLGETACTIVEATTRIBPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    typedef void (APIENTRY* PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    typedef void (APIENTRY* PFNGLGETATTACHEDSHADERSPROC) (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
    typedef GLint (APIENTRY* PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar *name);
    typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
    typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    typedef void (APIENTRY* PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
    typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    typedef void (APIENTRY* PFNGLGETSHADERSOURCEPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
    typedef GLint (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
    typedef void (APIENTRY* PFNGLGETUNIFORMFVPROC) (GLuint program, GLint location, GLfloat *params);
    typedef void (APIENTRY* PFNGLGETUNIFORMIVPROC) (GLuint program, GLint location, GLint *params);
    typedef void (APIENTRY* PFNGLGETVERTEXATTRIBDVPROC) (GLuint index, GLenum pname, GLdouble *params);
    typedef void (APIENTRY* PFNGLGETVERTEXATTRIBFVPROC) (GLuint index, GLenum pname, GLfloat *params);
    typedef void (APIENTRY* PFNGLGETVERTEXATTRIBIVPROC) (GLuint index, GLenum pname, GLint *params);
    typedef void (APIENTRY* PFNGLGETVERTEXATTRIBPOINTERVPROC) (GLuint index, GLenum pname, GLvoid* *pointer);
    typedef GLboolean (APIENTRY* PFNGLISPROGRAMPROC) (GLuint program);
    typedef GLboolean (APIENTRY* PFNGLISSHADERPROC) (GLuint shader);
    typedef void (APIENTRY* PFNGLLINKPROGRAMPROC) (GLuint program);
    typedef void (APIENTRY* PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
    typedef void (APIENTRY* PFNGLUSEPROGRAMPROC) (GLuint program);
    typedef void (APIENTRY* PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
    typedef void (APIENTRY* PFNGLUNIFORM2FPROC) (GLint location, GLfloat v0, GLfloat v1);
    typedef void (APIENTRY* PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    typedef void (APIENTRY* PFNGLUNIFORM4FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    typedef void (APIENTRY* PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
    typedef void (APIENTRY* PFNGLUNIFORM2IPROC) (GLint location, GLint v0, GLint v1);
    typedef void (APIENTRY* PFNGLUNIFORM3IPROC) (GLint location, GLint v0, GLint v1, GLint v2);
    typedef void (APIENTRY* PFNGLUNIFORM4IPROC) (GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    typedef void (APIENTRY* PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint *value);
    typedef void (APIENTRY* PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint *value);
    typedef void (APIENTRY* PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint *value);
    typedef void (APIENTRY* PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint *value);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    typedef void (APIENTRY* PFNGLVALIDATEPROGRAMPROC) (GLuint program);
    typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);

    GLAPI PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
    GLAPI PFNGLDRAWBUFFERSPROC glDrawBuffers;
    GLAPI PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate;
    GLAPI PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate;
    GLAPI PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate;
    GLAPI PFNGLATTACHSHADERPROC glAttachShader;
    GLAPI PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
    GLAPI PFNGLCOMPILESHADERPROC glCompileShader;
    GLAPI PFNGLCREATEPROGRAMPROC glCreateProgram;
    GLAPI PFNGLCREATESHADERPROC glCreateShader;
    GLAPI PFNGLDELETEPROGRAMPROC glDeleteProgram;
    GLAPI PFNGLDELETESHADERPROC glDeleteShader;
    GLAPI PFNGLDETACHSHADERPROC glDetachShader;
    GLAPI PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
    GLAPI PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
    GLAPI PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib;
    GLAPI PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
    GLAPI PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders;
    GLAPI PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
    GLAPI PFNGLGETPROGRAMIVPROC glGetProgramiv;
    GLAPI PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
    GLAPI PFNGLGETSHADERIVPROC glGetShaderiv;
    GLAPI PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
    GLAPI PFNGLGETSHADERSOURCEPROC glGetShaderSource;
    GLAPI PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
    GLAPI PFNGLGETUNIFORMFVPROC glGetUniformfv;
    GLAPI PFNGLGETUNIFORMIVPROC glGetUniformiv;
    GLAPI PFNGLGETVERTEXATTRIBDVPROC glGetVertexAttribdv;
    GLAPI PFNGLGETVERTEXATTRIBFVPROC glGetVertexAttribfv;
    GLAPI PFNGLGETVERTEXATTRIBIVPROC glGetVertexAttribiv;
    GLAPI PFNGLGETVERTEXATTRIBPOINTERVPROC glGetVertexAttribPointerv;
    GLAPI PFNGLISPROGRAMPROC glIsProgram;
    GLAPI PFNGLISSHADERPROC glIsShader;
    GLAPI PFNGLLINKPROGRAMPROC glLinkProgram;
    GLAPI PFNGLSHADERSOURCEPROC glShaderSource;
    GLAPI PFNGLUSEPROGRAMPROC glUseProgram;
    GLAPI PFNGLUNIFORM1FPROC glUniform1f;
    GLAPI PFNGLUNIFORM2FPROC glUniform2f;
    GLAPI PFNGLUNIFORM3FPROC glUniform3f;
    GLAPI PFNGLUNIFORM4FPROC glUniform4f;
    GLAPI PFNGLUNIFORM1IPROC glUniform1i;
    GLAPI PFNGLUNIFORM2IPROC glUniform2i;
    GLAPI PFNGLUNIFORM3IPROC glUniform3i;
    GLAPI PFNGLUNIFORM4IPROC glUniform4i;
    GLAPI PFNGLUNIFORM1FVPROC glUniform1fv;
    GLAPI PFNGLUNIFORM2FVPROC glUniform2fv;
    GLAPI PFNGLUNIFORM3FVPROC glUniform3fv;
    GLAPI PFNGLUNIFORM4FVPROC glUniform4fv;
    GLAPI PFNGLUNIFORM1IVPROC glUniform1iv;
    GLAPI PFNGLUNIFORM2IVPROC glUniform2iv;
    GLAPI PFNGLUNIFORM3IVPROC glUniform3iv;
    GLAPI PFNGLUNIFORM4IVPROC glUniform4iv;
    GLAPI PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
    GLAPI PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
    GLAPI PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
    GLAPI PFNGLVALIDATEPROGRAMPROC glValidateProgram;
    GLAPI PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
#endif  // GL_VERSION_2_0

// GL 2.1
#ifndef GL_VERSION_2_1
    #define GL_VERSION_2_1 1

    #define GL_CURRENT_RASTER_SECONDARY_COLOR   0x845F
    #define GL_PIXEL_PACK_BUFFER                0x88EB
    #define GL_PIXEL_UNPACK_BUFFER              0x88EC
    #define GL_PIXEL_PACK_BUFFER_BINDING        0x88ED
    #define GL_PIXEL_UNPACK_BUFFER_BINDING      0x88EF
    #define GL_FLOAT_MAT2x3                     0x8B65
    #define GL_FLOAT_MAT2x4                     0x8B66
    #define GL_FLOAT_MAT3x2                     0x8B67
    #define GL_FLOAT_MAT3x4                     0x8B68
    #define GL_FLOAT_MAT4x2                     0x8B69
    #define GL_FLOAT_MAT4x3                     0x8B6A
    #define GL_SRGB                             0x8C40
    #define GL_SRGB8                            0x8C41
    #define GL_SRGB_ALPHA                       0x8C42
    #define GL_SRGB8_ALPHA8                     0x8C43
    #define GL_COMPRESSED_SRGB_ALPHA            0x8C49

    typedef void (APIENTRY* PFNGLUNIFORMMATRIX2X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX3X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX2X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX4X2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX3X4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    typedef void (APIENTRY* PFNGLUNIFORMMATRIX4X3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

    GLAPI PFNGLUNIFORMMATRIX2X3FVPROC glUniformMatrix2x3fv;
    GLAPI PFNGLUNIFORMMATRIX3X2FVPROC glUniformMatrix3x2fv;
    GLAPI PFNGLUNIFORMMATRIX2X4FVPROC glUniformMatrix2x4fv;
    GLAPI PFNGLUNIFORMMATRIX4X2FVPROC glUniformMatrix4x2fv;
    GLAPI PFNGLUNIFORMMATRIX3X4FVPROC glUniformMatrix3x4fv;
    GLAPI PFNGLUNIFORMMATRIX4X3FVPROC glUniformMatrix4x3fv;
#endif  // GL_VERSION_2_1

// GL 3.0
#ifndef GL_VERSION_3_0
    #define GL_VERSION_3_0
    #define GL_NUM_EXTENSIONS                   0x821D

    typedef const GLubyte * (APIENTRY* PFNGLGETSTRINGIPROC) (GLenum, GLuint);
    GLAPI PFNGLGETSTRINGIPROC glGetStringi;
#endif // GL_VERSION_3_0

// Extensions
// EXT_framebuffer_object
#ifndef GL_EXT_framebuffer_object
    #define GL_EXT_framebuffer_object 1

    #define GL_INVALID_FRAMEBUFFER_OPERATION_EXT                0x0506
    #define GL_MAX_RENDERBUFFER_SIZE_EXT                        0x84E8
    #define GL_FRAMEBUFFER_BINDING_EXT                          0x8CA6
    #define GL_RENDERBUFFER_BINDING_EXT                         0x8CA7
    #define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT           0x8CD0
    #define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT           0x8CD1
    #define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT         0x8CD2
    #define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT 0x8CD3
    #define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT    0x8CD4
    #define GL_FRAMEBUFFER_COMPLETE_EXT                         0x8CD5
    #define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT            0x8CD6
    #define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT    0x8CD7
    #define GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT  0x8CD8
    #define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT            0x8CD9
    #define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT               0x8CDA
    #define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT           0x8CDB
    #define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT           0x8CDC
    #define GL_FRAMEBUFFER_UNSUPPORTED_EXT                      0x8CDD
    #define GL_MAX_COLOR_ATTACHMENTS_EXT                        0x8CDF
    #define GL_COLOR_ATTACHMENT0_EXT                            0x8CE0
    #define GL_COLOR_ATTACHMENT1_EXT                            0x8CE1
    #define GL_COLOR_ATTACHMENT2_EXT                            0x8CE2
    #define GL_COLOR_ATTACHMENT3_EXT                            0x8CE3
    #define GL_COLOR_ATTACHMENT4_EXT                            0x8CE4
    #define GL_COLOR_ATTACHMENT5_EXT                            0x8CE5
    #define GL_COLOR_ATTACHMENT6_EXT                            0x8CE6
    #define GL_COLOR_ATTACHMENT7_EXT                            0x8CE7
    #define GL_COLOR_ATTACHMENT8_EXT                            0x8CE8
    #define GL_COLOR_ATTACHMENT9_EXT                            0x8CE9
    #define GL_COLOR_ATTACHMENT10_EXT                           0x8CEA
    #define GL_COLOR_ATTACHMENT11_EXT                           0x8CEB
    #define GL_COLOR_ATTACHMENT12_EXT                           0x8CEC
    #define GL_COLOR_ATTACHMENT13_EXT                           0x8CED
    #define GL_COLOR_ATTACHMENT14_EXT                           0x8CEE
    #define GL_COLOR_ATTACHMENT15_EXT                           0x8CEF
    #define GL_DEPTH_ATTACHMENT_EXT                             0x8D00
    #define GL_STENCIL_ATTACHMENT_EXT                           0x8D20
    #define GL_FRAMEBUFFER_EXT                                  0x8D40
    #define GL_RENDERBUFFER_EXT                                 0x8D41
    #define GL_RENDERBUFFER_WIDTH_EXT                           0x8D42
    #define GL_RENDERBUFFER_HEIGHT_EXT                          0x8D43
    #define GL_RENDERBUFFER_INTERNAL_FORMAT_EXT                 0x8D44
    #define GL_STENCIL_INDEX1_EXT                               0x8D46
    #define GL_STENCIL_INDEX4_EXT                               0x8D47
    #define GL_STENCIL_INDEX8_EXT                               0x8D48
    #define GL_STENCIL_INDEX16_EXT                              0x8D49
    #define GL_RENDERBUFFER_RED_SIZE_EXT                        0x8D50
    #define GL_RENDERBUFFER_GREEN_SIZE_EXT                      0x8D51
    #define GL_RENDERBUFFER_BLUE_SIZE_EXT                       0x8D52
    #define GL_RENDERBUFFER_ALPHA_SIZE_EXT                      0x8D53
    #define GL_RENDERBUFFER_DEPTH_SIZE_EXT                      0x8D54
    #define GL_RENDERBUFFER_STENCIL_SIZE_EXT                    0x8D55

    typedef GLboolean (APIENTRY* PFNGLISRENDERBUFFEREXTPROC) (GLuint renderbuffer);
    typedef void (APIENTRY* PFNGLBINDRENDERBUFFEREXTPROC) (GLenum target, GLuint renderbuffer);
    typedef void (APIENTRY* PFNGLDELETERENDERBUFFERSEXTPROC) (GLsizei n, const GLuint *renderbuffers);
    typedef void (APIENTRY* PFNGLGENRENDERBUFFERSEXTPROC) (GLsizei n, GLuint *renderbuffers);
    typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEEXTPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    typedef void (APIENTRY* PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC) (GLenum target, GLenum pname, GLint *params);
    typedef GLboolean (APIENTRY* PFNGLISFRAMEBUFFEREXTPROC) (GLuint framebuffer);
    typedef void (APIENTRY* PFNGLBINDFRAMEBUFFEREXTPROC) (GLenum target, GLuint framebuffer);
    typedef void (APIENTRY* PFNGLDELETEFRAMEBUFFERSEXTPROC) (GLsizei n, const GLuint *framebuffers);
    typedef void (APIENTRY* PFNGLGENFRAMEBUFFERSEXTPROC) (GLsizei n, GLuint *framebuffers);
    typedef GLenum (APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) (GLenum target);
    typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE1DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE3DEXTPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    typedef void (APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    typedef void (APIENTRY* PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC) (GLenum target, GLenum attachment, GLenum pname, GLint *params);
    typedef void (APIENTRY* PFNGLGENERATEMIPMAPEXTPROC) (GLenum target);

    GLAPI PFNGLISRENDERBUFFEREXTPROC glIsRenderbufferEXT;
    GLAPI PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
    GLAPI PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
    GLAPI PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
    GLAPI PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
    GLAPI PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC glGetRenderbufferParameterivEXT;
    GLAPI PFNGLISFRAMEBUFFEREXTPROC glIsFramebufferEXT;
    GLAPI PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
    GLAPI PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
    GLAPI PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
    GLAPI PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
    GLAPI PFNGLFRAMEBUFFERTEXTURE1DEXTPROC glFramebufferTexture1DEXT;
    GLAPI PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
    GLAPI PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
    GLAPI PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
    GLAPI PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC glGetFramebufferAttachmentParameterivEXT;
    GLAPI PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;
#endif

// EXT_texture_filter_anisotropic
#ifndef GL_EXT_texture_filter_anisotropic
    #define GL_EXT_texture_filter_anisotropic 1

    #define GL_TEXTURE_MAX_ANISOTROPY_EXT       0x84FE
    #define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT   0x84FF
#endif

// EXT_texture_compression_s3tc
#ifndef GL_EXT_texture_compression_s3tc
    #define GL_EXT_texture_compression_s3tc 1

    #define GL_COMPRESSED_RGB_S3TC_DXT1_EXT     0x83F0
    #define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT    0x83F1
    #define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT    0x83F2
    #define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT    0x83F3
#endif

// EXT_texture_sRGB
#ifndef GL_EXT_texture_sRGB
    #define GL_EXT_texture_sRGB 1

    #define GL_SRGB_EXT                            0x8C40
    #define GL_SRGB8_EXT                           0x8C41
    #define GL_SRGB_ALPHA_EXT                      0x8C42
    #define GL_SRGB8_ALPHA8_EXT                    0x8C43
    //#define GL_SLUMINANCE_ALPHA_EXT                 0x8C44
    //#define GL_SLUMINANCE8_ALPHA8_EXT               0x8C45
    //#define GL_SLUMINANCE_EXT                       0x8C46
    //#define GL_SLUMINANCE8_EXT                      0x8C47
    #define GL_COMPRESSED_SRGB_EXT                  0x8C48
    #define GL_COMPRESSED_SRGB_ALPHA_EXT            0x8C49
    //#define GL_COMPRESSED_SLUMINANCE_EXT            0x8C4A
    //#define GL_COMPRESSED_SLUMINANCE_ALPHA_EXT      0x8C4B
    #define GL_COMPRESSED_SRGB_S3TC_DXT1_EXT        0x8C4C
    #define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT  0x8C4D
    #define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT  0x8C4E
    #define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT  0x8C4F
#endif

// ARB_texture_float
#ifndef GL_ARB_texture_float
    #define GL_ARB_texture_float 1

    #define GL_TEXTURE_RED_TYPE_ARB             0x8C10
    #define GL_TEXTURE_GREEN_TYPE_ARB           0x8C11
    #define GL_TEXTURE_BLUE_TYPE_ARB            0x8C12
    //#define GL_TEXTURE_ALPHA_TYPE_ARB           0x8C13
    //#define GL_TEXTURE_LUMINANCE_TYPE_ARB       0x8C14
    //#define GL_TEXTURE_INTENSITY_TYPE_ARB       0x8C15
    #define GL_TEXTURE_DEPTH_TYPE_ARB           0x8C16
    #define GL_UNSIGNED_NORMALIZED_ARB          0x8C17
    #define GL_RGBA32F_ARB                      0x8814
    #define GL_RGB32F_ARB                       0x8815
    //#define GL_ALPHA32F_ARB                     0x8816
    //#define GL_INTENSITY32F_ARB                 0x8817
    //#define GL_LUMINANCE32F_ARB                 0x8818
    //#define GL_LUMINANCE_ALPHA32F_ARB           0x8819
    #define GL_RGBA16F_ARB                      0x881A
    #define GL_RGB16F_ARB                       0x881B
    //#define GL_ALPHA16F_ARB                     0x881C
    //#define GL_INTENSITY16F_ARB                 0x881D
    //#define GL_LUMINANCE16F_ARB                 0x881E
    //#define GL_LUMINANCE_ALPHA16F_ARB           0x881F
#endif

// EXT_framebuffer_blit
#ifndef GL_EXT_framebuffer_blit
    #define GL_EXT_framebuffer_blit 1

    #define GL_READ_FRAMEBUFFER_EXT             0x8CA8
    #define GL_DRAW_FRAMEBUFFER_EXT             0x8CA9
    #define GL_READ_FRAMEBUFFER_BINDING_EXT     GL_FRAMEBUFFER_BINDING_EXT
    #define GL_DRAW_FRAMEBUFFER_BINDING_EXT     0x8CAA

    typedef void (APIENTRY* PFNGLBLITFRAMEBUFFEREXTPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

    GLAPI PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT;
#endif

// EXT_framebuffer_multisample
#ifndef GL_EXT_framebuffer_multisample
    #define GL_EXT_framebuffer_multisample 1

    #define GL_RENDERBUFFER_SAMPLES_EXT                0x8CAB
    #define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT  0x8D56
    #define GL_MAX_SAMPLES_EXT                         0x8D57

    typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

    GLAPI PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
#endif

// ARB_timer_query
#ifndef GL_ARB_timer_query
    #define GL_ARB_timer_query 1

    #define GL_TIME_ELAPSED  0x88BF
    #define GL_TIMESTAMP     0x8E28

    typedef void (APIENTRY* PFNGLQUERYCOUNTERPROC) (GLuint id, GLenum target);
    typedef void (APIENTRY* PFNGLGETQUERYOBJECTI64VPROC) (GLuint id, GLenum pname, GLint64 *params);
    typedef void (APIENTRY* PFNGLGETQUERYOBJECTUI64VPROC) (GLuint id, GLenum pname, GLuint64 *params);
    GLAPI PFNGLQUERYCOUNTERPROC glQueryCounter;
    GLAPI PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64v;
    GLAPI PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64v;
#endif

#endif
