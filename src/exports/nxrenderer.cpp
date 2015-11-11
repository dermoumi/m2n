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
static bool initialized = false;
static uint32_t vlShape;
static uint32_t vbTriangle;
static uint32_t defaultShader;
static uint32_t rbShader;
static uint32_t texture;
static uint32_t rbTex;
static uint32_t rb;

#include "../graphics/opengl.hpp"

static uint32_t colorTex;
static uint32_t fb;
static uint32_t depthRb;

//----------------------------------------------------------
NX_EXPORT void nxRendererTestRelease()
{
    if (!initialized) return;

    glDeleteTextures(1, &colorTex);
    glDeleteRenderbuffersEXT(1, &depthRb);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glDeleteFramebuffersEXT(1, &fb);

    rdi->destroyShader(defaultShader);
    rdi->destroyShader(rbShader);
    rdi->destroyBuffer(vbTriangle);
    rdi->destroyTexture(texture);
    rdi->destroyRenderBuffer(rb);

    initialized = false;
}

//----------------------------------------------------------
NX_EXPORT void nxRendererTestInit()
{
    nxRendererTestRelease();

    // Register Vertex layouts
    RenderDevice::VertexLayoutAttrib attribs[] = {
        {"vertPos", 0, 2, 0},
        {"texCoords0", 0, 2, 8}
    };
    vlShape = rdi->registerVertexLayout(2, attribs);

    // Create tiangle
    float verts[4*4] {
        -0.5f, -0.5f, 0.f, 0.f,
         0.5f, -0.5f, 1.f, 0.f,
        -0.5f,  0.5f, 0.f, 1.f,
         0.5f,  0.5f, 1.f, 1.f
    };
    vbTriangle = rdi->createVertexBuffer(4 * 4 * sizeof(float), verts);

    // Create shaders
    defaultShader = rdi->createShader(
        "#version 120\n"
        "attribute vec2 vertPos;\n"
        "attribute vec2 texCoords0;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "   coords = texCoords0;\n"
        "   gl_Position = vec4(vertPos, 0.0, 1.0);\n"
        "}\n",
        "#version 120\n"
        "uniform sampler2D tex;\n"
        "varying vec2 coords;\n"
        // "layout(location = 0) out vec3 color;\n"
        "void main() {\n"
        "   vec4 col = texture2D(tex, coords);\n"
        "   gl_FragColor = col * vec4(1.0, 1.0, 0.0, 1.0);\n"
        // "   color = col * vec4(1.0, 1.0, 0.0, 1.0);\n"
        "}\n"
    );

    rbShader = rdi->createShader(
        "attribute vec2 vertPos;\n"
        "attribute vec2 texCoords0;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "   coords = texCoords0;\n"
        "   gl_Position = vec4(vertPos, 0.0, 1.0);\n"
        "}\n",
        "uniform sampler2D tex;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "   vec4 col = texture2D(tex, coords);\n"
        "   gl_FragColor = col;\n"
        "}\n"
    );

    // Create render buffer
    rb = rdi->createRenderBuffer(1024, 1024, RenderDevice::RGBA8, false, 1, 0);
    rbTex = rdi->getRenderBufferTexture(rb, 0);

    // Load image
    Image img;
    if (!img.open("assets/pasrien.png")) {
        Log::error("Could not load image");
        return;
    }

    unsigned int imgWidth, imgHeight;
    img.getSize(&imgWidth, &imgHeight);

    // Create texture
    // unsigned char texData[] = 
    //     { 128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
    //       128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
    //       128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255,
    //       128,192,255,255, 128,192,255,255, 128,192,255,255, 128,192,255,255 };

    texture = rdi->createTexture(RenderDevice::Tex2D, imgWidth, imgHeight, 1, RenderDevice::RGBA8,
        true, true, false);
    rdi->uploadTextureData(texture, 0, 0, nullptr);
    rdi->uploadTextureSubData(texture, 0, 0, 0, 0, 0, 512, 512, 1, img.getPixelsPtr());

    // texture = 0;

    // Get texture data
    // uint8_t* buffer = new uint8_t[imgWidth * imgHeight * 4];
    // rdi->getTextureData(texture, 0, 0, buffer);
    // Image img2;
    // img.create(imgWidth, imgHeight, buffer);
    // img.save("testimgD.png");

    // glGenTextures(1, &colorTex);
    // glBindTexture(GL_TEXTURE_2D, colorTex);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 256, 256, 0, GL_BGRA, GL_UNSIGNED_BYTE, nullptr);

    // glGenFramebuffersEXT(1, &fb);
    // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    // glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, colorTex, 0);

    // glGenRenderbuffersEXT(1, &depthRb);
    // glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthRb);
    // glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, 256, 256);
    // glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthRb);

    // GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    // if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
    //     Log::info("Couldn't create framebuffer");
    // }
    // else {
    //     Log::info("Framebuffer created successfully");
    // }

    rdi->resetStates();

    rdi->beginRendering();

    static float color[] = {1.f, 1.f, 0.5f, 1.f};

    rdi->setRenderBuffer(rb);

    // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    rdi->setViewport(0, 0, 1024, 1024);
    rdi->clear(color);

    rbTex = rdi->getRenderBufferTexture(rb, 0);

    uint8_t* buffer = new uint8_t[1024 * 1024 * 4];
    rdi->getTextureData(rbTex, 0, 0, buffer);
    Image img2;
    img.create(1024, 1024, buffer);
    img.save("testimgD.png");

    rdi->setRenderBuffer(0);
    
    rdi->finishRendering();


    initialized = true;
    Log::info("loaded %u %u %u %u %u", rb, rbTex, texture, defaultShader);
}

//----------------------------------------------------------
NX_EXPORT void nxRendererTestRender()
{
    static float color[] = {1.f, 1.f, 1.f, 1.f};

    rdi->setRenderBuffer(rb);

    // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
    rdi->setViewport(0, 0, 1024, 1024);
    rdi->clear(color);

    // rdi->setTexture(1, texture, RenderDevice::FilterTrilinear | RenderDevice::Aniso8 |
    //     RenderDevice::AddrWrap);

    // rdi->bindShader(defaultShader);

    // // Affect texture to shader
    // auto loc = rdi->getShaderSamplerLoc(defaultShader, "tex");
    // // Log::info("Loc2: %i", loc);
    // if (loc >= 0) rdi->setShaderSampler(loc, 1);

    // rdi->draw(RenderDevice::TriangleStrip, 0, 4);

    // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    rdi->setRenderBuffer(0);
    rdi->setViewport(0, 0, 1280, 720);

    rdi->setVertexBuffer(0, vbTriangle, 0, 16);
    rdi->setIndexBuffer( 0, RenderDevice::U16 );
    rdi->setVertexLayout(vlShape);

    rbTex = rdi->getRenderBufferTexture(rb, 0);
    rdi->setTexture(1, rbTex, RenderDevice::FilterTrilinear | RenderDevice::Aniso8 |
        RenderDevice::AddrWrap);

    rdi->bindShader(rbShader);
    rdi->setBlendMode(true, RenderDevice::SrcAlpha, RenderDevice::InvSrcAlpha);

    // Affect texture to shader
    auto loc2 = rdi->getShaderSamplerLoc(rbShader, "tex");
    // Log::info("Loc2: %i", loc);
    if (loc2 >= 0) rdi->setShaderSampler(loc2, 1);

    rdi->draw(RenderDevice::TriangleStrip, 0, 4);
}

//==============================================================================
