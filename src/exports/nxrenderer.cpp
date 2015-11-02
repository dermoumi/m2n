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
#include "../config.hpp"
#include "../system/log.hpp"

#if !defined(NX_OPENGL_ES)
    #include "../graphics/renderdevicegl.hpp"
#else
    #include "../graphics/renderdevicegles2.hpp"
#endif

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
static RenderDevice* rdi {nullptr};
    
//----------------------------------------------------------
// Exported functions
//----------------------------------------------------------
NX_EXPORT bool nxRendererInit()
{
    // Delete the render device if reinitiazing
    if (rdi) delete rdi;

    // Instanciate the proper render device
    #if !defined(NX_OPENGL_ES)
        rdi = new RenderDeviceGL();
    #else
        rdi = new RenderDeviceGLES2();
    #endif

    return rdi->initialize();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    float clearColor[] = {r/255.f, g/255.f, b/255.f, a/255.f};

    rdi->clear(clearColor);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererBegin()
{
    rdi->beginRendering();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererFinish()
{
    rdi->finishRendering();
}

//----------------------------------------------------------
NX_EXPORT void nxRendererSetupViewport(int x, int y, int width, int height)
{
    rdi->setViewport(x, y, width, height);
    // rdi->setScissorRect(x, y, width, height);
}

//----------------------------------------------------------
static uint32_t vlShape;
static uint32_t vbTriangle;
static uint32_t defaultShader;

//----------------------------------------------------------
NX_EXPORT void nxRendererTestInit()
{
    // Register Vertex layouts
    VertexLayoutAttrib attribsPosOnly[1] {
        {"vertPos", 0, 3, 0}
    };
    vlShape = rdi->registerVertexLayout(1, attribsPosOnly);

    float triVerts[3*3] {
         0.f,   0.5f, 0.f,
        -0.5f, -0.5f, 0.f,
         0.5f, -0.5f, 0.f
    };
    vbTriangle = rdi->createVertexBuffer(3 * 3 * sizeof(float), triVerts);

    defaultShader = rdi->createShader(
        "attribute vec3 vertPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(vertPos, 1.0);\n"
        "}\n",
        "void main() {\n"
        "   gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
        "}\n"
    );
}

//----------------------------------------------------------
NX_EXPORT void nxRendererTestRender()
{
    rdi->bindShader(defaultShader);

    rdi->setVertexBuffer(0, vbTriangle, 0, 12);
    rdi->setIndexBuffer( 0, IDXFMT_16 );
    rdi->setVertexLayout(vlShape);

    rdi->draw(PRIM_TRIANGLES, 0, 3);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererTestRelease()
{
    rdi->destroyShader(defaultShader);
    rdi->destroyBuffer(vbTriangle);
}

//==============================================================================
