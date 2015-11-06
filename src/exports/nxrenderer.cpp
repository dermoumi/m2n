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
#include "../system/log.hpp"
#include "../graphics/image.hpp"
static uint32_t vlShape;
static uint32_t vbTriangle;
static uint32_t defaultShader;
static uint32_t texture;

//----------------------------------------------------------
NX_EXPORT void nxRendererTestRelease()
{
    rdi->destroyShader(defaultShader);
    rdi->destroyBuffer(vbTriangle);
    rdi->destroyTexture(texture);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererTestInit()
{
    nxRendererTestRelease();

    // Register Vertex layouts
    VertexLayoutAttrib attribsPosOnly[] = {
        {"vertPos", 0, 2, 0},
        {"texCoords", 0, 2, 8}
    };
    vlShape = rdi->registerVertexLayout(1, attribsPosOnly);

    // Create tiangle
    float verts[4*4] {
        -0.5f, -0.5f, 0.f, 0.f,
        -0.5f,  0.5f, 0.f, 0.f,
         0.5f, -0.5f, 0.f, 0.f,
         0.5f,  0.5f, 0.f, 0.f
    };
    vbTriangle = rdi->createVertexBuffer(4 * 4 * sizeof(float), verts);

    // Create shaders
    defaultShader = rdi->createShader(
        "#version 120\n"
        "attribute vec2 vertPos;\n"
        "attribute vec2 texCoords;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "   coords = texCoords;\n"
        "   gl_Position = vec4(vertPos, 0.0, 1.0);\n"
        "}\n",
        "#version 120\n"
        "uniform sampler2D tex;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "   vec4 col = texture2D(tex, coords);\n"
        "   gl_FragColor = vec4(0.0, 0.0, col.b, 0.5);\n"
        "}\n"
    );

    Log::info("shader: %u", defaultShader);

    // Load image
    Image img;
    if (!img.open("assets/pasrien.png")) {
        Log::error("Could not load image");
        return;
    }

    unsigned int imgWidth, imgHeight;
    img.getSize(&imgWidth, &imgHeight);
    Log::info("sizes: %u %u", imgWidth, imgHeight);

    // Create texture
    unsigned char texData[] = 
        { 128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
          128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
          128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
          128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255 };

    texture = rdi->createTexture(TextureType::Tex2D, 4, 4, 1, TextureFormat::RGBA8,
        true, true, false);
    Log::info("texture: %u", texture);
    rdi->uploadTextureData(texture, 0, 0, texData);

    // Get texture data
    // uint8_t* buffer = new uint8_t[imgWidth * imgHeight * 4];
    // rdi->getTextureData(texture, 0, 0, buffer);
    // Image img2;
    // img.create(imgWidth, imgHeight, buffer);
    // img.save("testimgD.png");

}

//----------------------------------------------------------
NX_EXPORT void nxRendererTestRender()
{
    rdi->setTexture(1, texture, SamplerState::FilterTrilinear | SamplerState::Aniso8 |
        SamplerState::AddrWrap);

    rdi->bindShader(defaultShader);

    // Affect texture to shader
    auto loc = rdi->getShaderSamplerLoc(defaultShader, "tex");
    // Log::info("Loc2: %i", loc);
    if (loc >= 0) rdi->setShaderSampler(loc, 1);


    rdi->setVertexBuffer(0, vbTriangle, 0, 16);
    rdi->setIndexBuffer( 0, IDXFMT_16 );
    rdi->setVertexLayout(vlShape);

    rdi->draw(PRIM_TRISTRIP, 0, 4);
}

//==============================================================================
