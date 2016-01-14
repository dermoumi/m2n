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

#include "renderdevicegles2.hpp"

#if defined(NX_OPENGL_ES)
#include "../system/log.hpp"
#include "opengles2.hpp"

#include <mutex>

// Locals
thread_local const char* defaultShaderVS =
    "uniform mat4 viewProjMat;\n"
    "uniform mat4 worldMat;\n"
    "attribute vec3 vertPos;\n"
    "void main() {\n"
    "    gl_Position = viewProjMat * worldMat * vec4(vertPos, 1.0);\n"
    "}\n";

thread_local const char* defaultShaderFS =
    "uniform mediump vec4 color;\n"
    "void main() {\n"
    "   gl_FragColor = color;\n"
    "}\n";

thread_local GLenum toVertexFormat[] = {GL_FLOAT, GL_UNSIGNED_BYTE};
thread_local GLenum toIndexFormat[]  = {GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};
thread_local GLenum toTexType[]      = {GL_TEXTURE_2D, GL_TEXTURE_3D_OES, GL_TEXTURE_CUBE_MAP};
thread_local GLenum toTexBinding[]   = {
    GL_TEXTURE_BINDING_2D, GL_TEXTURE_BINDING_3D_OES, GL_TEXTURE_BINDING_CUBE_MAP
};
thread_local GLenum toPrimType[]     = {
    GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_STRIP,
    GL_TRIANGLE_FAN
};

static       std::mutex  vlMutex;
thread_local std::string shaderLog;

bool RenderDeviceGLES2::initialize()
{
    bool failed {false};

    std::string vendor   {(const char*)(glGetString(GL_VENDOR))};
    std::string renderer {(const char*)(glGetString(GL_RENDERER))};
    std::string version  {(const char*)(glGetString(GL_VERSION))};
    Log::info("Initializing OpenGL ES2 Backend using OpenGL driver '" + version + "'"
        " by '" + vendor + "' on '" + renderer + "'");

    // Initialize extensions
    if (!initOpenGLExtensions()) {
        Log::error("Could not find all required OpenGL function entry points");
        failed = true;
    }

    // Check that OpenGL 2.0 is available
    if (glExt::majorVersion * 10 + glExt::minorVersion < 21) {
        Log::error("OpenGL ES 2.1 is not available");
        failed = true;
    }

    // Check that required extensions are supported
    if (!glExt::EXT_texture_filter_anisotropic) {
        Log::warning("Extension EXT_texture_filter_anisotropic not supported");
    }

    // Something went wrong
    if (failed) {
        Log::fatal("Failed to init renderer backend, debug info following");

        std::string extensions {reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS))};
        Log::info("Supported OpenGL extensions: '" + extensions + "'");

        return false;
    }

    // Get the currently bound framebuffer
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mDefaultFBO);

    // Get capabilities
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &mMaxCubeTextureSize);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &mMaxTextureUnits);
    if (mMaxTextureUnits > 16) mMaxTextureUnits = 16;

    mMaxColBuffers = 1;

    mDXTSupported = glExt::EXT_texture_compression_s3tc || (glExt::EXT_texture_compression_dxt1 &&
        glExt::ANGLE_texture_compression_dxt3 && glExt::ANGLE_texture_compression_dxt5);
    mPVRTCISupported = glExt::IMG_texture_compression_pvrtc;
    mTexETC1Supported = glExt::OES_compressed_ETC1_RGB8_texture;

    mTexFloatSupported = false;
    mTexDepthSupported = glExt::OES_depth_texture || glExt::ANGLE_depth_texture;
    mTexShadowSamplers = glExt::EXT_shadow_samplers;

    mTex3DSupported = glExt::OES_texture_3D;
    mTexNPOTSupported = false;
    mTexSRGBSupported = false;

    mRTMultiSampling = glExt::ANGLE_framebuffer_blit || glExt::EXT_multisampled_render_to_texture;
    if( glExt::EXT_multisampled_render_to_texture ) {
        glExt::ANGLE_framebuffer_blit = false;
        glExt::ANGLE_framebuffer_multisample = false;
    }

    mOccQuerySupported = glExt::EXT_occlusion_query_boolean;
    mTimerQuerySupported = glExt::EXT_disjoint_timer_query;

    // Set some default values
    mIndexFormat = GL_UNSIGNED_SHORT;
    mActiveVertexAttribsMask = 0u;

    mDepthFormat = GL_DEPTH_COMPONENT16;

    initStates();
    resetStates();

    return true;
}

void RenderDeviceGLES2::initStates()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void RenderDeviceGLES2::resetStates()
{
    mCurIndexBuffer = reinterpret_cast<IndexBuffer*>(1u); mNewIndexBuffer = nullptr;
    mCurVertexLayout = 1;                                 mNewVertexLayout = 0;
    mCurRasterState.hash = 0xFFFFFFFFu;                   mNewRasterState.hash = 0u;
    mCurBlendState.hash = 0xFFFFFFFFu;                    mNewBlendState.hash = 0u;
    mCurDepthStencilState.hash = 0xFFFFFFFFu;             mCurDepthStencilState.hash = 0u;

    for (uint32_t i = 0; i < 16; ++i) setTexture(i, 0, 0);

    setColorWriteMask(true);
    mPendingMask = 0xFFFFFFFFu;
    mVertexBufUpdated = true;
    commitStates();

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);
}

bool RenderDeviceGLES2::commitStates(uint32_t filter)
{
    uint32_t mask = mPendingMask & filter;
    if (mask) {
        // Set viewport
        if (mask & Viewport) {
            glViewport(mVpX, mVpY, mVpWidth, mVpHeight);
            mPendingMask &= ~Viewport;
        }

        // Update renderstates
        if (mask & RenderStates) {
            applyRenderStates();
            mPendingMask &= ~RenderStates;
        }

        // Set scissor rect
        if (mask & Scissor)
        {
            glScissor(mScX, mScY, mScWidth, mScHeight);
            mPendingMask &= ~Scissor;
        }

        // Bind index buffer
        if (mask & IndexBuffers) {
            if (mNewIndexBuffer != mCurIndexBuffer) {
                glBindBuffer(
                    GL_ELEMENT_ARRAY_BUFFER,
                    mNewIndexBuffer ? static_cast<IndexBufferGLES2*>(mNewIndexBuffer)->mHandle : 0
                );

                mCurIndexBuffer = mNewIndexBuffer;
            }

            mPendingMask &= ~IndexBuffers;
        }

        // Bind textures and set sampler state
        if (mask & Textures) {
            for (int i = 0; i < mMaxTextureUnits; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                auto& texSlot = mTexSlots[i];

                if (texSlot.texObj == 0) {
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    glBindTexture(GL_TEXTURE_3D_OES, 0);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
                else {
                    auto& tex = mTextures.getRef(texSlot.texObj);
                    glBindTexture(tex.type, tex.glObj);

                    // Apply sampler state
                    if (tex.samplerState != texSlot.samplerState) {
                        tex.samplerState = texSlot.samplerState;
                        applySamplerState(tex);
                    }
                }
            }

            mPendingMask &= ~Textures;
        }

        // Bind vertex buffers
        if (mask & VertexLayouts) {
            if (
                mNewVertexLayout != mCurVertexLayout || mPrevShader != mCurShader ||
                mVertexBufUpdated
            ) {
                if (!applyVertexLayout()) return false;

                mCurVertexLayout  = mNewVertexLayout;
                mPrevShader       = mCurShader;
                mVertexBufUpdated = false;
            }

            mPendingMask &= ~VertexLayouts;
        }
    }

    return true;
}

void RenderDeviceGLES2::clear(uint32_t flags, const float* color, float depth)
{
    if (mCurRenderBuffer != 0) {
        auto& rb = mRenderBuffers.getRef(mCurRenderBuffer);

        if ((flags & ClrDepth) && rb.depthTex == 0) flags &= ~ClrDepth;
    }

    uint32_t oglClearMask = 0;

    if (flags & ClrDepth) {
        oglClearMask |= GL_DEPTH_BUFFER_BIT;
        glClearDepthf(depth);
    }

    if (flags & (ClrColorRT0 | ClrColorRT1 | ClrColorRT2 | ClrColorRT3)) {
        oglClearMask |= GL_COLOR_BUFFER_BIT;

        if (color) {
            glClearColor(color[0], color[1], color[2], color[3]);
        }
        else {
            glClearColor(0.f, 0.f, 0.f, 0.f);
        }
    }

    if (oglClearMask) {
        commitStates(Viewport | Scissor | RenderStates);
        glClear(oglClearMask);
    }
}

void RenderDeviceGLES2::draw(PrimType primType, uint32_t firstVert, uint32_t vertCount)
{
    if (commitStates()) {
        glDrawArrays(toPrimType[primType], firstVert, vertCount);
    }
}

void RenderDeviceGLES2::drawIndexed(PrimType primType, uint32_t firstIndex, uint32_t indexCount)
{
    if (commitStates()) {
        firstIndex *= (mIndexFormat == GL_UNSIGNED_SHORT) ? sizeof(short) : sizeof(int);

        glDrawElements(toPrimType[primType], indexCount, mIndexFormat, (char*)0 + firstIndex);
    }
}

void RenderDeviceGLES2::beginRendering()
{
    // Get the currently bound frame buffer object.
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mDefaultFBO);

    mCurIndexBuffer = reinterpret_cast<IndexBuffer*>(1u); mNewIndexBuffer = nullptr;
    mCurVertexLayout = 1;                                 mNewVertexLayout = 0;
    mCurRasterState.hash = 0xFFFFFFFFu;                   mNewRasterState.hash = 0u;
    mCurBlendState.hash = 0xFFFFFFFFu;                    mNewBlendState.hash = 0u;
    mCurDepthStencilState.hash = 0xFFFFFFFFu;             mCurDepthStencilState.hash = 0u;

    setColorWriteMask(true);
    mVertexBufUpdated = true;
    commitStates();

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);
}

void RenderDeviceGLES2::finishRendering()
{
    // Nothing to do
}

uint32_t RenderDeviceGLES2::registerVertexLayout(uint8_t numAttribs,
    const VertexLayoutAttrib* attribs)
{
    if (mNumVertexLayouts == MaxNumVertexLayouts) return 0;

    std::lock_guard<std::mutex> lock(vlMutex);
    mVertexLayouts[mNumVertexLayouts].numAttribs = numAttribs;
    for (uint8_t i = 0; i < numAttribs; ++i) {
        mVertexLayouts[mNumVertexLayouts].attribs[i] = attribs[i];
    }

    return ++mNumVertexLayouts;
}

uint32_t RenderDeviceGLES2::createTexture(TextureType type, int width, int height,
    unsigned int depth, TextureFormat format, bool hasMips, bool genMips, bool sRGB)
{
    if (!mTexNPOTSupported && ((width & (width-1)) != 0 || ((height & (height-1)) != 0))) {
        Log::error("Non-Power-Of-Two textures are not supported by GPU");
        return 0;
    }

    if (type == TexCube && (width > mMaxCubeTextureSize || height > mMaxTextureSize)) {
        Log::error("Cube map is bigger than limit size");
        return 0;
    }
    else if (type != TexCube && (width > mMaxTextureSize || height > mMaxTextureSize ||
        depth > static_cast<unsigned int>(mMaxTextureSize)))
    {
        Log::error("Texture is bigger than limit size");
        return 0;
    }

    if (!mDXTSupported && (format == DXT1 || format == DXT3 || format == DXT5)) {
        Log::error("S3TC/DXT texture formats are not supported by the GPU");
        return 0;
    }

    if (!mPVRTCISupported && (format == PVRTCI_2BPP || format == PVRTCI_A2BPP ||
        format == PVRTCI_4BPP || format == PVRTCI_A4BPP))
    {
        Log::error("PVRTCI texture formats are not supported by the GPU");
        return 0;
    }

    if (!mTexETC1Supported && format == ETC1) {
        Log::error("ETC1 texture format is not supported by the GPU");
        return 0;
    }

    if (!mTex3DSupported && type == Tex3D) {
        Log::error("3D Textures are not supported by the GPU");
        return 0;
    }

    RDITexture tex;
    tex.type    = toTexType[type];
    tex.format  = format;
    tex.width   = width;
    tex.height  = height;
    tex.depth   = depth;
    tex.sRGB    = sRGB;
    tex.genMips = genMips;
    tex.hasMips = hasMips;

    switch(format) {
    case RGBA8:
        tex.glFmt = GL_RGBA;
        break;
    case DXT1:
        tex.glFmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case DXT3:
        tex.glFmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case DXT5:
        tex.glFmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    case RGBA16F:
        tex.glFmt = GL_RGBA;
        break;
    case RGBA32F:
        tex.glFmt = GL_RGBA;
        break;
    case PVRTCI_2BPP:
        tex.glFmt = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        break;
    case PVRTCI_A2BPP:
        tex.glFmt = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        break;
    case PVRTCI_4BPP:
        tex.glFmt = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        break;
    case PVRTCI_A4BPP:
        tex.glFmt = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        break;
    case ETC1:
        tex.glFmt = GL_ETC1_RGB8_OES;
        break;
    case DEPTH:
        tex.glFmt = GL_DEPTH_COMPONENT;
        break;
    default:
        Log::warning("Could not create texture: invalid format");
        return 0;
    }

    glGenTextures(1, &tex.glObj);
    if (tex.glObj == 0) return 0;

    glActiveTexture(GL_TEXTURE0);

    int lastTexture;
    glGetIntegerv(toTexBinding[tex.type], &lastTexture);

    glBindTexture(tex.type, tex.glObj);

    tex.samplerState = 0;
    applySamplerState(tex);

    glBindTexture(tex.type, lastTexture);

    // Calculate memory requirements
    tex.memSize = calcTextureSize(format, width, height, depth);
    if (hasMips || genMips) tex.memSize += static_cast<int>(tex.memSize / 3.f + 0.5f);
    if (type == TextureType::TexCube) tex.memSize *= 6;
    mTextureMemory += tex.memSize;

    return mTextures.add(tex);
}

void RenderDeviceGLES2::uploadTextureData(uint32_t texObj, int slice, int mipLevel,
    const void* pixels)
{
    if (texObj == 0) return;

    const auto& tex = mTextures.getRef(texObj);
    auto format     = tex.format;

    int inputFormat = GL_RGBA;
    int inputType   = GL_UNSIGNED_BYTE;
    bool compressed = (format == DXT1) || (format == DXT3) || (format == DXT5) ||
        (format == ETC1) || (format == PVRTCI_2BPP) || (format == PVRTCI_A2BPP) ||
        (format == PVRTCI_4BPP) || (format == PVRTCI_A4BPP);

    glActiveTexture(GL_TEXTURE0);

    int lastTexture;
    glGetIntegerv(toTexBinding[tex.type], &lastTexture);

    glBindTexture(tex.type, tex.glObj);

    switch (format) {
        case RGBA16F:
        case RGBA32F:
            inputType   = GL_FLOAT;
            break;
        case DEPTH:
            inputFormat = GL_DEPTH_COMPONENT;
            inputType   = GL_UNSIGNED_SHORT;
            break;
        default:
            break;
    }

    // Calculate size of the next mipmap using "floor" convention
    int width = std::max(tex.width >> mipLevel, 1);
    int height = std::max(tex.height >> mipLevel, 1);

    if (tex.type == GL_TEXTURE_2D || tex.type == GL_TEXTURE_CUBE_MAP) {
        int target = (tex.type == GL_TEXTURE_2D) ?
            GL_TEXTURE_2D : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice);

        if (compressed) {
            glCompressedTexImage2D(target, mipLevel, tex.glFmt, width, height, 0,
                calcTextureSize(format, width, height, 1), pixels);
        }
        else {
            glTexImage2D(target, mipLevel, tex.glFmt, width, height, 0, inputFormat, inputType,
                pixels);
        }
    }
    else if (tex.type == GL_TEXTURE_3D_OES) {
        int depth = std::max(tex.depth >> mipLevel, 1);

        if (compressed) {
            glCompressedTexImage3DOES(GL_TEXTURE_3D_OES, mipLevel, tex.glFmt, width, height, depth,
                0, calcTextureSize(format, width, height, depth), pixels);
        }
        else {
            glTexImage3DOES(GL_TEXTURE_3D_OES, mipLevel, tex.glFmt, width, height, depth, 0,
                inputFormat, inputType, pixels);
        }
    }

    if (tex.genMips && (tex.type != GL_TEXTURE_CUBE_MAP || slice == 5)) {
        // Note: cube map mips are only generated when the last side is uploaded
        glGenerateMipmap(tex.type);
    }

    glBindTexture(tex.type, lastTexture);
}

void RenderDeviceGLES2::uploadTextureSubData(uint32_t texObj, int slice, int mipLevel,
    unsigned int x, unsigned int y, unsigned int z, unsigned int width, unsigned int height,
    unsigned int depth, const void* pixels)
{
    if (texObj == 0) return;

    const auto& tex = mTextures.getRef(texObj);
    auto format     = tex.format;

    if (
        x + width > static_cast<unsigned int>(tex.width) ||
        y + height > static_cast<unsigned int>(tex.height)
    ) {
        Log::info("Attempting to update portion out of texture boundaries");
        return;
    }

    glActiveTexture(GL_TEXTURE0);

    int lastTexture;
    glGetIntegerv(toTexBinding[tex.type], &lastTexture);

    glBindTexture(tex.type, tex.glObj);

    int inputFormat = GL_RGBA;
    int inputType   = GL_UNSIGNED_BYTE;
    bool compressed = (format == DXT1) || (format == DXT3) || (format == DXT5);

    switch (format) {
        case RGBA16F:
        case RGBA32F:
            inputType = GL_FLOAT;
            break;
        case DEPTH:
            inputFormat = GL_DEPTH_COMPONENT;
            inputType = GL_UNSIGNED_SHORT;
            break;
        default:
            break;
    }

    if (tex.type == GL_TEXTURE_2D || tex.type == GL_TEXTURE_CUBE_MAP) {
        int target = (tex.type == GL_TEXTURE_2D) ?
            GL_TEXTURE_2D : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice);

        if (compressed) {
            glCompressedTexSubImage2D(target, mipLevel, x, y, width, height, tex.glFmt,
                calcTextureSize(format, width, height, 1), pixels);
        }
        else {
            glTexSubImage2D(target, mipLevel, x, y, width, height, inputFormat, inputType, pixels);
        }
    }
    else if (tex.type == GL_TEXTURE_3D_OES) {
        if (compressed) {
            glCompressedTexSubImage3DOES(GL_TEXTURE_3D_OES, mipLevel, x, y, z, width, height, depth,
                tex.glFmt, calcTextureSize(format, width, height, depth), pixels);
        }
        else {
            glTexSubImage3DOES(GL_TEXTURE_3D_OES, mipLevel, x, y, z, width, height, depth,
                inputFormat, inputType, pixels);
        }
    }

    if (tex.genMips && (tex.type != GL_TEXTURE_CUBE_MAP || slice == 5)) {
        // Note: cube map mips are only generated when the last side is uploaded
        glGenerateMipmap(tex.type);
    }

    glBindTexture(tex.type, lastTexture);
}

void RenderDeviceGLES2::destroyTexture(uint32_t texObj)
{
    if (texObj == 0) return;
    const auto& tex = mTextures.getRef(texObj);

    glDeleteTextures(1, &tex.glObj);
    mTextureMemory -= tex.memSize;
    mTextures.remove(texObj);
}

bool RenderDeviceGLES2::getTextureData(uint32_t texObj, int slice, int mipLevel, void* buffer)
{
    const auto& tex = mTextures.getRef(texObj);

    if (tex.format != RGBA8 || tex.type == GL_TEXTURE_3D_OES) {
        // Can only retrieve RGBA8 2D data
        return false;
    }

    int target = (tex.type == GL_TEXTURE_CUBE_MAP) ?
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice : GL_TEXTURE_2D;

    GLuint fb {0u};
    glGenFramebuffers(1, &fb);
    if (!fb) return false;

    GLint prevFb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFb);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, tex.glObj, mipLevel);
    glReadPixels(0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    glDeleteFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFb);
    return false;
}

uint32_t RenderDeviceGLES2::getTextureMemory() const
{
    return mTextureMemory;
}

Shader* RenderDeviceGLES2::newShader()
{
    return new ShaderGLES2(this);
}

void RenderDeviceGLES2::bind(Shader* shader)
{
    glUseProgram(shader ? static_cast<ShaderGLES2*>(shader)->mHandle : 0);

    mCurShader    = shader;
    mPendingMask |= VertexLayouts;
}

const std::string& RenderDeviceGLES2::getShaderLog()
{
    return shaderLog;
}

const char* RenderDeviceGLES2::getDefaultVSCode()
{
    return defaultShaderVS;
}

const char* RenderDeviceGLES2::getDefaultFSCode()
{
    return defaultShaderFS;
}

Shader* RenderDeviceGLES2::getCurrentShader() const
{
    return mCurShader;
}

VertexBuffer* RenderDeviceGLES2::newVertexBuffer()
{
    return new VertexBufferGLES2(this);
}

uint32_t RenderDeviceGLES2::usedVertexBufferMemory() const
{
    return mVertexBufferMemory;
}

void RenderDeviceGLES2::bind(VertexBuffer* buffer, uint8_t slot, uint32_t offset)
{
    auto& vbSlot = mVertBufSlots[slot];
    if (vbSlot.vbObj != buffer || vbSlot.offset != offset) {
        vbSlot = {buffer, offset};
        mVertexBufUpdated = true;
        mPendingMask |= VertexLayouts;
    }
}

IndexBuffer* RenderDeviceGLES2::newIndexBuffer()
{
    return new IndexBufferGLES2(this);
}

uint32_t RenderDeviceGLES2::usedIndexBufferMemory() const
{
    return mIndexBufferMemory;
}

void RenderDeviceGLES2::bind(IndexBuffer* buffer)
{
    mIndexFormat = toIndexFormat[buffer ? buffer->format() : 0];
    mNewIndexBuffer = buffer;
    mPendingMask |= IndexBuffers;
}

uint32_t RenderDeviceGLES2::createRenderBuffer(uint32_t width, uint32_t height,
    TextureFormat format, bool depth, uint32_t numColBufs, uint32_t samples)
{
    if ((format == RGBA16F || format == RGBA32F) && !mTexFloatSupported) {
        return 0;
    }

    if (numColBufs > static_cast<unsigned int>(mMaxColBuffers)) {
        return 0;
    }

    uint32_t maxSamples = 0;
    if (glExt::EXT_multisampled_render_to_texture || glExt::ANGLE_framebuffer_multisample) {
        GLint value;
        glGetIntegerv(glExt::EXT_multisampled_render_to_texture ?
            GL_MAX_SAMPLES_EXT : GL_MAX_SAMPLES_ANGLE, &value);
        maxSamples = static_cast<uint32_t>(value);
    }

    if (samples > maxSamples)
    {
        samples = maxSamples;
        Log::warning("GPU does not support desired multisampling quality for render target");
    }

    RDIRenderBuffer rb;
    rb.width = width;
    rb.height = height;
    rb.samples = samples;

    // Create framebuffers
    glGenFramebuffers(1, &rb.fbo);
    if (samples > 0 && glExt::ANGLE_framebuffer_multisample) glGenFramebuffers(1, &rb.fboMS);

    if (numColBufs > 0) {
        // Attach color buffers
        for (uint32_t i = 0; i < numColBufs; ++i) {
            glBindFramebuffer(GL_FRAMEBUFFER, rb.fbo);

            // Create a color texture
            uint32_t texObj = createTexture(Tex2D, rb.width, rb.height, 1, format, false, false,
                false);
            uploadTextureData(texObj, 0, 0, nullptr);
            rb.colTexs[i] = texObj;

            // Attach the texture
            auto& tex = mTextures.getRef(texObj);
            if (samples > 0 && glExt::EXT_multisampled_render_to_texture) {
                glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                    GL_TEXTURE_2D, tex.glObj, 0, samples);
            }
            else {
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                    tex.glObj, 0);
            }

            if (samples > 0 && glExt::ANGLE_framebuffer_multisample) {
                uint32_t glFmt = 0;
                if (format == RGBA8 && glExt::OES_rgb8_rgba8) glFmt = GL_RGBA8_OES;

                if (glFmt != 0) {
                    glBindFramebuffer(GL_FRAMEBUFFER, rb.fboMS);

                    // Create a multisampled renderbuffer
                    glGenRenderbuffers(1, &rb.colBufs[i]);
                    glBindRenderbuffer(GL_RENDERBUFFER, rb.colBufs[i]);
                    glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, rb.samples, glFmt,
                        rb.width, rb.height);

                    // Attach the renderbuffer
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                        GL_RENDERBUFFER, rb.colBufs[i]);
                }
                else {
                    Log::error("GPU does not support multisampling for this format");
                }
            }
        }
    }

    if (depth) {
        glBindFramebuffer(GL_FRAMEBUFFER, rb.fbo);

        // Create depth texture
        if ((samples > 0 && glExt::EXT_multisampled_render_to_texture) ||
            (!glExt::OES_depth_texture && !glExt::ANGLE_depth_texture))
        {            
            glGenRenderbuffers(1, &rb.depthBuf);
            glBindRenderbuffer(GL_RENDERBUFFER, rb.depthBuf);

            if (samples > 0 && glExt::EXT_multisampled_render_to_texture) {
                glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, samples, mDepthFormat,
                    rb.width, rb.height);
            }
            else {
                glRenderbufferStorage(GL_RENDERBUFFER, mDepthFormat, rb.width, rb.height);
            }

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                rb.depthBuf);
        }
        else {
            Log::info("test2");

            uint32_t texObj = createTexture(Tex2D, rb.width, rb.height, 1, DEPTH, false, false,
                false);
            Log::info("test3");
            if (glExt::EXT_shadow_samplers) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE);
            }

            uploadTextureData(texObj, 0, 0, nullptr);
            rb.depthTex = texObj;
            auto& tex = mTextures.getRef(texObj);

            // Attach the texture
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                tex.glObj, 0);
        }

        if (samples > 0 && glExt::ANGLE_framebuffer_multisample) {
            glBindFramebuffer(GL_FRAMEBUFFER, rb.fboMS);

            // Create a multisampled renderbuffer
            glGenRenderbuffers(1, &rb.depthBufMS);
            glBindRenderbuffer(GL_RENDERBUFFER, rb.depthBufMS);
            glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, rb.samples, mDepthFormat,
                rb.width, rb.height);

            // Attach the render buffer
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                rb.depthBufMS);
        }
    }

    uint32_t rbObj = mRenderBuffers.add(rb);

    // Check if FBO is cmplete
    glBindFramebuffer(GL_FRAMEBUFFER, rb.fbo);
    uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    bool valid = (status == GL_FRAMEBUFFER_COMPLETE);

    if (samples > 0 && glExt::ANGLE_framebuffer_multisample) {
        glBindFramebuffer(GL_FRAMEBUFFER, rb.fboMS);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        if (status != GL_FRAMEBUFFER_COMPLETE) valid = false;
    }

    if (!valid)
    {
        destroyRenderBuffer(rbObj);
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);

    return rbObj;
}

void RenderDeviceGLES2::destroyRenderBuffer(uint32_t rbObj)
{
    auto& rb = mRenderBuffers.getRef(rbObj);

    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);

    if (rb.depthTex != 0) destroyTexture(rb.depthTex);
    if (rb.depthBuf != 0) glDeleteRenderbuffers(1, &rb.depthBuf);
    if (rb.depthBufMS != 0) glDeleteRenderbuffers(1, &rb.depthBufMS);
    rb.depthTex = rb.depthBuf = rb.depthBufMS = 0;

    for (uint32_t i = 0; i < RDIRenderBuffer::MaxColorAttachmentCount; ++i) {
        if (rb.colTexs[i] != 0) destroyTexture(rb.colTexs[i]);
        if (rb.colBufs[i] != 0) glDeleteRenderbuffers(1, &rb.colBufs[i]);
        rb.colTexs[i] = rb.colBufs[i] = 0;
    }

    if (rb.fbo != 0) glDeleteFramebuffers(1, &rb.fbo);
    if (rb.fboMS != 0) glDeleteFramebuffers(1, &rb.fboMS);
    rb.fbo = rb.fboMS = 0;

    mRenderBuffers.remove(rbObj);
}

uint32_t RenderDeviceGLES2::getRenderBufferTexture(uint32_t rbObj, uint32_t bufIndex)
{
    auto& rb = mRenderBuffers.getRef(rbObj);

    if (bufIndex < static_cast<uint32_t>(mMaxColBuffers)) return rb.colTexs[bufIndex];
    if (bufIndex == 32) return rb.depthTex;

    return 0;
}

void RenderDeviceGLES2::setRenderBuffer(uint32_t rbObj)
{
    // Resolve reneder buffer if necessary
    if (mCurRenderBuffer != 0) resolveRenderBuffer(mCurRenderBuffer);

    // Set new render buffer
    mCurRenderBuffer = rbObj;

    if (rbObj == 0) {
        // Check if the default render buffer is already bound, since this
        // call can be extremely expensive on some platforms

        int currentFrameBuffer;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFrameBuffer);

        if (currentFrameBuffer != mDefaultFBO) {
            glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);
        }

        mFbWidth  = mVpX + mVpWidth;
        mFbHeight = mVpY + mVpHeight;
    }
    else {
        // Unbind all textures to make sure that n FBO attachment is bound anymore
        for (int i = 0; i < mMaxTextureUnits; ++i) {
            setTexture(i, 0, 0);
        }

        commitStates(Textures);

        auto& rb = mRenderBuffers.getRef(rbObj);

        glBindFramebuffer(GL_FRAMEBUFFER, rb.fboMS != 0 ? rb.fboMS : rb.fbo);

        mFbWidth  = rb.width;
        mFbHeight = rb.height;
    }
}

void RenderDeviceGLES2::getRenderBufferSize(uint32_t rbObj, int* width, int* height)
{
    if (rbObj == 0) {
        if (width)  *width  = mVpWidth;
        if (height) *height = mVpHeight;
    }
    else {
        auto& rb = mRenderBuffers.getRef(rbObj);

        if (width)  *width  = rb.width;
        if (height) *height = rb.height;
    }
}

bool RenderDeviceGLES2::getRenderBufferData(uint32_t rbObj, int bufIndex, int* width, int* height,
    int* compCount, void* dataBuffer, int bufferSize)
{
    int x, y, w, h;
    int format = GL_RGBA;
    int type = GL_UNSIGNED_BYTE;

    beginRendering();
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    if (rbObj == 0) {
        if (bufIndex != 32 && bufIndex != 0) return false;

        if (width)  *width = mVpWidth;
        if (height) *height = mVpHeight;

        x = mVpX;
        y = mVpY;
        w = mVpWidth;
        h = mVpHeight;

        glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);
    }
    else {
        resolveRenderBuffer(rbObj);
        auto& rb = mRenderBuffers.getRef(rbObj);

        if (bufIndex == 32 && rb.depthTex == 0) return false;

        if (bufIndex != 32 && (rb.colTexs[bufIndex] == 0 ||
            static_cast<unsigned int>(bufIndex) >= RDIRenderBuffer::MaxColorAttachmentCount))
        {
            return false;
        }

            if (width)  *width = rb.width;
            if (height) *height = rb.height;

            x = 0;
            y = 0;
            w = rb.width;
            h = rb.height;

            glBindFramebuffer(GL_FRAMEBUFFER, rb.fbo);
    }

    glFinish();

    if (bufIndex == 32) {
        format = GL_DEPTH_COMPONENT;
        type = GL_UNSIGNED_SHORT;
    }

    int comps = (bufIndex == 32 ? 1 : RDIRenderBuffer::MaxColorAttachmentCount);
    if (compCount) *compCount = comps;

    bool retVal = false;
    if (dataBuffer && bufferSize >= w * h * comps * (type == GL_UNSIGNED_SHORT ? 2 : 1)) {
        glReadPixels(x, y, w, h, format, type, dataBuffer);
        retVal = true;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);
    return retVal;
}

void RenderDeviceGLES2::setViewport(int x, int y, int width, int height)
{
    mVpX      = x;
    mVpY      = y;
    mVpWidth  = width;
    mVpHeight = height;

    mPendingMask |= Viewport;
}

void RenderDeviceGLES2::setScissorRect(int x, int y, int width, int height)
{
    mScX      = x;
    mScY      = y;
    mScWidth  = width;
    mScHeight = height;

    mPendingMask |= Scissor;
}

void RenderDeviceGLES2::setVertexLayout(uint32_t vlObj)
{
    mNewVertexLayout = vlObj;
}

void RenderDeviceGLES2::setTexture(uint32_t slot, uint32_t texObj, uint16_t samplerState)
{
    mTexSlots[slot] = {texObj, samplerState};
    mPendingMask |= Textures;
}

void RenderDeviceGLES2::setColorWriteMask(bool enabled)
{
    mNewRasterState.renderTargetWriteMask = enabled;
}

bool RenderDeviceGLES2::getColorWriteMask() const
{
    return mNewRasterState.renderTargetWriteMask;
}

void RenderDeviceGLES2::setFillMode(FillMode fillMode)
{
    mNewRasterState.fillMode = fillMode;
    mPendingMask |= RenderStates;
}

RenderDevice::FillMode RenderDeviceGLES2::getFillMode() const
{
    return static_cast<FillMode>(mNewRasterState.fillMode);
}

void RenderDeviceGLES2::setCullMode(CullMode cullMode)
{
    mNewRasterState.cullMode = cullMode;
    mPendingMask |= RenderStates;
}

RenderDevice::CullMode RenderDeviceGLES2::getCullMode() const
{
    return static_cast<CullMode>(mNewRasterState.cullMode);
}

void RenderDeviceGLES2::setScissorTest(bool enabled)
{
    mNewRasterState.scissorEnable = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGLES2::getScissorTest() const
{
    return mNewRasterState.scissorEnable;
}

void RenderDeviceGLES2::setMultisampling(bool enabled)
{
    mNewRasterState.multisampleEnable = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGLES2::getMultisampling() const
{
    return mNewRasterState.multisampleEnable;
}

void RenderDeviceGLES2::setAlphaToCoverage(bool enabled)
{
    mNewBlendState.alphaToCoverageEnable = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGLES2::getAlphaToCoverage() const
{
    return mNewBlendState.alphaToCoverageEnable;
}

void RenderDeviceGLES2::setBlendMode(bool enabled, BlendFunc src, BlendFunc dst)
{
    mNewBlendState.blendEnable = enabled;
    mNewBlendState.srcBlendFunc = src;
    mNewBlendState.dstBlendFunc = dst;

    mPendingMask |= RenderStates;
}

bool RenderDeviceGLES2::getBlendMode(BlendFunc& src, BlendFunc& dst) const
{
    src = static_cast<BlendFunc>(mNewBlendState.srcBlendFunc);
    dst = static_cast<BlendFunc>(mNewBlendState.dstBlendFunc);
    return mNewBlendState.blendEnable;
}

void RenderDeviceGLES2::setDepthMask(bool enabled)
{
    mNewDepthStencilState.depthWriteMask = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGLES2::getDepthMask() const
{
    return mNewDepthStencilState.depthWriteMask;
}

void RenderDeviceGLES2::setDepthTest(bool enabled)
{
    mNewDepthStencilState.depthEnable = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGLES2::getDepthTest() const
{
    return mNewDepthStencilState.depthEnable;
}

void RenderDeviceGLES2::setDepthFunc(DepthFunc depthFunc)
{
    mNewDepthStencilState.depthFunc = depthFunc;
    mPendingMask |= RenderStates;
}

RenderDevice::DepthFunc RenderDeviceGLES2::getDepthFunc() const
{
    return static_cast<DepthFunc>(mNewDepthStencilState.depthFunc);
}

void RenderDeviceGLES2::sync()
{
    glFinish();
}

void RenderDeviceGLES2::getCapabilities(uint32_t* maxTexUnits, uint32_t* maxTexSize,
        uint32_t* maxCubTexSize, uint32_t* maxColBufs, bool* dxt, bool* pvrtci, bool* etc1,
        bool* texFloat, bool* texDepth, bool* texSS, bool* tex3D, bool* texNPOT, bool* texSRGB,
        bool* rtms, bool* occQuery, bool* timerQuery, bool* multithreading) const
{
    if (maxTexUnits)    *maxTexUnits    = mMaxTextureUnits;
    if (maxTexSize)     *maxTexSize     = mMaxTextureSize;
    if (maxCubTexSize)  *maxCubTexSize  = mMaxCubeTextureSize;
    if (maxColBufs)     *maxColBufs     = mMaxColBuffers;
    if (dxt)            *dxt            = mDXTSupported;
    if (pvrtci)         *pvrtci         = mPVRTCISupported;
    if (etc1)           *etc1           = mTexETC1Supported;
    if (texFloat)       *texFloat       = mTexFloatSupported;
    if (texDepth)       *texDepth       = mTexDepthSupported;
    if (texSS)          *texSS          = mTexShadowSamplers;
    if (tex3D)          *tex3D          = mTex3DSupported;
    if (texNPOT)        *texNPOT        = mTexNPOTSupported;
    if (texSRGB)        *texSRGB        = mTexSRGBSupported;
    if (rtms)           *rtms           = mRTMultiSampling;
    if (occQuery)       *occQuery       = mOccQuerySupported;
    if (timerQuery)     *timerQuery     = mTimerQuerySupported;
    if (multithreading) {
        #if defined(NX_SYSTEM_IOS)
            // Needs testing?
            *multithreading = true;
        #else
            *multithreading = false;
        #endif
    }
}

bool RenderDeviceGLES2::applyVertexLayout()
{
    if (mNewVertexLayout != 0) {
        if (!mCurShader) return false;

        uint32_t newVertexAttribMask {0u};

        ShaderGLES2* shader         = static_cast<ShaderGLES2*>(mCurShader);
        RDIInputLayout& inputLayout = shader->mInputLayouts[mNewVertexLayout-1];

        if (!inputLayout.valid) return false;

        // Set vertex attrib pointers
        auto& vl = mVertexLayouts[mNewVertexLayout-1];
        for (uint32_t i = 0; i < vl.numAttribs; ++i) {
            int8_t attribIndex = inputLayout.attribIndices[i];
            if (attribIndex >= 0) {
                VertexLayoutAttrib& attrib = vl.attribs[i];
                const auto& vbSlot = mVertBufSlots[attrib.vbSlot];

                VertexBufferGLES2* buffer = static_cast<VertexBufferGLES2*>(vbSlot.vbObj);
                if (buffer) {
                    GLenum format = toVertexFormat[attrib.format];
                    glBindBuffer(GL_ARRAY_BUFFER, buffer->mHandle);
                    glVertexAttribPointer(
                        attribIndex, attrib.size, format, format == GL_UNSIGNED_BYTE,
                        buffer->stride(), (char*)0 + vbSlot.offset + attrib.offset
                    );
                }
                else {
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                newVertexAttribMask |= 1 << attribIndex;
            }
        }

        // Enable/Disable active vertex attribute arrays
        for (uint32_t i = 0; i < 16u; ++i) {
            uint32_t curBit = 1 << i;
            if ((newVertexAttribMask & curBit) != (mActiveVertexAttribsMask & curBit)) {
                if (newVertexAttribMask & curBit) {
                    glEnableVertexAttribArray(i);
                }
                else {
                    glDisableVertexAttribArray(i);
                }
            }
        }
        mActiveVertexAttribsMask = newVertexAttribMask;
    }

    return true;
}

void RenderDeviceGLES2::applySamplerState(RDITexture& tex)
{
    thread_local const uint32_t magFilters[] = {GL_LINEAR, GL_LINEAR, GL_NEAREST};
    thread_local const uint32_t minFiltersMips[] = {
        GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST
    };
    thread_local const uint32_t maxAniso[] = {1u, 2u, 4u, 8u, 16u, 1u, 1u, 1u};
    thread_local const uint32_t wrapModes[] = {GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_EDGE};

    uint32_t state = tex.samplerState;
    uint32_t target = tex.type;

    auto filter = (state & FilterMask) >> FilterStart;
    glTexParameteri(
        target, GL_TEXTURE_MIN_FILTER, tex.hasMips ? minFiltersMips[filter] : magFilters[filter]
    );

    filter = (state & FilterMask) >> FilterStart;
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilters[filter]);

    if (glExt::EXT_texture_filter_anisotropic) {
        filter = (state & AnisoMask) >> AnisoStart;
        glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso[filter]);
    }

    filter = (state & AddrUMask) >> AddrUStart;
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapModes[filter]);

    filter = (state & AddrVMask) >> AddrVStart;
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapModes[filter]);

    if (mTex3DSupported) {
        filter = (state & AddrWMask) >> AddrWStart;
        glTexParameteri(target, GL_TEXTURE_WRAP_R_OES, wrapModes[filter]);
    }

    if (mTexShadowSamplers) {
        if (!(state & CompLEqual)) {
            glTexParameteri(target, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE);
        }
        else {
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL);
        }
    }
}

void RenderDeviceGLES2::applyRenderStates()
{
    // Rasterizer state
    if (mNewRasterState.hash != mCurRasterState.hash) {
        // Not supported on GLES2
        // if (mNewRasterState.fillMode == RS_FILL_SOLID) {
        //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // }
        // else {
        //     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // }

        if (mNewRasterState.cullMode == Back) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else if (mNewRasterState.cullMode == Front) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
        }
        else {
            glDisable(GL_CULL_FACE);
        }

        if (!mNewRasterState.scissorEnable) {
            glDisable(GL_SCISSOR_TEST);
        }
        else {
            glEnable(GL_SCISSOR_TEST);
        }

        if (mNewRasterState.renderTargetWriteMask) {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
        else {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        }

        mCurRasterState.hash = mNewRasterState.hash;
    }

    // Blend state
    if (mNewBlendState.hash != mCurBlendState.hash) {
        if (!mNewBlendState.alphaToCoverageEnable) {
            glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
        else {
            glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }

        if (!mNewBlendState.blendEnable) {
            glDisable(GL_BLEND);
        }
        else {
            static uint32_t oglBlendFuncs[] = {
                GL_ZERO,
                GL_ONE,
                GL_SRC_ALPHA,
                GL_ONE_MINUS_SRC_ALPHA,
                GL_DST_COLOR
            };

            glEnable(GL_BLEND);
            glBlendFunc(
                oglBlendFuncs[mNewBlendState.srcBlendFunc],
                oglBlendFuncs[mNewBlendState.dstBlendFunc]
            );
        }

        mCurBlendState.hash = mNewBlendState.hash;
    }

    // Depth-stencil state
    if (mNewDepthStencilState.hash != mCurDepthStencilState.hash) {
        if (mNewDepthStencilState.depthWriteMask) {
            glDepthMask(GL_TRUE);
        }
        else {
            glDepthMask(GL_FALSE);
        }

        if (mNewDepthStencilState.depthEnable) {
            static uint32_t oglDepthFuncs[] = {
                GL_LEQUAL,
                GL_LESS,
                GL_EQUAL,
                GL_GREATER,
                GL_GEQUAL,
                GL_ALWAYS
            };

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(oglDepthFuncs[mNewDepthStencilState.depthFunc]);
        }
        else {
            glDisable(GL_DEPTH_TEST);
        }

        mCurDepthStencilState.hash = mNewDepthStencilState.hash;
    }
}

void RenderDeviceGLES2::resolveRenderBuffer(uint32_t rbObj)
{
    auto& rb = mRenderBuffers.getRef(rbObj);

    // Only needed when using ANGLE_framebuffer_multisample (which uses fboMS)
    if (rb.fboMS == 0) return;

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, rb.fboMS);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, rb.fbo);

    bool depthResolved = false;
    for (uint32_t i = 0; i < RDIRenderBuffer::MaxColorAttachmentCount; ++i) {
        if (rb.colBufs[i] != 0) {
            int mask = GL_COLOR_BUFFER_BIT;
            if (
                !depthResolved && rb.depthBufMS != 0 && rb.depthTex != 0 &&
                !glExt::ANGLE_depth_texture
            ) {
                //Cannot resolve depth textures created with ANGLE_depth_texture
                mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
                depthResolved = true;
            }

            glBlitFramebufferANGLE(
                0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height, mask, GL_NEAREST
            );
        }
    }

    if (!depthResolved && rb.depthBufMS != 0 && rb.depthTex != 0 && !glExt::ANGLE_depth_texture) {
        //Cannot resolve depth textures created with ANGLE_depth_texture
        glBlitFramebufferANGLE(
            0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height,
            GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
        );
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mDefaultFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mDefaultFBO);
}

RenderDeviceGLES2::ShaderGLES2::ShaderGLES2(RenderDeviceGLES2* device) :
    mDevice(device)
{
    for (uint32_t i = 0; i < MaxNumVertexLayouts; ++i) {
        memset(&mInputLayouts[i], 0, sizeof(RDIInputLayout));
    }
}

RenderDeviceGLES2::ShaderGLES2::~ShaderGLES2()
{
    if (mHandle) glDeleteProgram(mHandle);
}

bool RenderDeviceGLES2::ShaderGLES2::load(const char* vertexShader, const char* fragmentShader)
{
    int infoLogLength {0};
    int charsWritten  {0};
    char* infoLog     {nullptr};
    int status;

    shaderLog = "";

    // Vertex shader
    uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, nullptr);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if (!status) {
        // Get info
        glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 1) {
            infoLog = new char[infoLogLength];
            glGetShaderInfoLog(vs, infoLogLength, &charsWritten, infoLog);
            shaderLog = shaderLog + "[Vertex Shader]\n" + infoLog;
            delete[] infoLog;
            infoLog = nullptr;
        }

        glDeleteShader(vs);
        return false;
    }

    // Fragment shader
    uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, nullptr);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
    if (!status) {
        glGetShaderiv(fs, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (infoLogLength > 1) {
            infoLog = new char[infoLogLength];
            glGetShaderInfoLog(fs, infoLogLength, &charsWritten, infoLog);
            shaderLog = shaderLog + "[Fragment Shader]\n" + infoLog;
            delete[] infoLog;
            infoLog = nullptr;
        }

        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    // Shader program
    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Link shader program
    shaderLog = "";

    glLinkProgram(program);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 1) {
        infoLog = new char[infoLogLength];
        glGetProgramInfoLog(program, infoLogLength, &charsWritten, infoLog);
        shaderLog = shaderLog + "[Linking]\n" + infoLog;
        delete[] infoLog;
        infoLog = nullptr;
    }

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        glDeleteProgram(program);
        return false;
    }

    int attribCount;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attribCount);

    // Run through vertex layouts and check which is compatible with this shader
    std::lock_guard<std::mutex> lock(vlMutex);
    for (uint32_t i = 0; i < mDevice->mNumVertexLayouts; ++i) {
        bool allAttribsFound = true;
        auto& vl = mDevice->mVertexLayouts[i];

        // Reset attribute indices to -1 (no attribute)
        for (uint32_t j = 0; j < 16u; ++j) {
            mInputLayouts[i].attribIndices[j] = -1;
        }

        // Check if shader has all declared attributes, and set locations
        for (int j = 0; j < attribCount; ++j)
        {
            char name[32];
            uint32_t size, type;
            glGetActiveAttrib(program, j, 32, nullptr, (int*)&size, &type, name);

            bool attribFound = false;
            for (uint32_t k = 0; k < vl.numAttribs; ++k) {
                if (vl.attribs[k].semanticName == name) {
                    auto loc = glGetAttribLocation(program, name);
                    mInputLayouts[i].attribIndices[k] = loc;
                    attribFound = true;
                }
            }

            if (!attribFound) {
                allAttribsFound = false;
                break;
            }
        }

        // An input layout is only valid for this shader if all attributes were found
        mInputLayouts[i].valid = allAttribsFound;
    }

    mHandle = program;
    return true;
}

void RenderDeviceGLES2::ShaderGLES2::setUniform(int location, uint8_t type, float* data,
    uint32_t count)
{
    switch(type) {
    case Float:
        glUniform1fv(location, count, data);
        break;
    case Float2:
        glUniform2fv(location, count, data);
        break;
    case Float3:
        glUniform3fv(location, count, data);
        break;
    case Float4:
        glUniform4fv(location, count, data);
        break;
    case Float44:
        glUniformMatrix4fv(location, count, false, data);
        break;
    case Float33:
        glUniformMatrix3fv(location, count, false, data);
        break;
    }
}

void RenderDeviceGLES2::ShaderGLES2::setSampler(int location, uint8_t unit)
{
    glUniform1i(location, static_cast<int>(unit));
}

int RenderDeviceGLES2::ShaderGLES2::uniformLocation(const char* name) const
{
    return glGetUniformLocation(mHandle, name);
}

int RenderDeviceGLES2::ShaderGLES2::samplerLocation(const char* name) const
{
    return glGetUniformLocation(mHandle, name);
}

RenderDeviceGLES2::VertexBufferGLES2::VertexBufferGLES2(RenderDeviceGLES2* device) :
    mDevice(device)
{
    // Nothing else to do
}

RenderDeviceGLES2::VertexBufferGLES2::~VertexBufferGLES2()
{
    release();
}

bool RenderDeviceGLES2::VertexBufferGLES2::load(void* data, uint32_t size, uint32_t stride)
{
    release();

    glGenBuffers(1, &mHandle);
    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mDevice->mVertexBufferMemory += size;
    mSize = size;
    mStride = stride;
    return true;
}

bool RenderDeviceGLES2::VertexBufferGLES2::update(void* data, uint32_t size, uint32_t offset)
{
    if (offset + size > mSize) return false;

    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    if (offset == 0u && size == mSize) {
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }
    else {
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    }

    return true;
}

uint32_t RenderDeviceGLES2::VertexBufferGLES2::size() const
{
    return mSize;
}

uint32_t RenderDeviceGLES2::VertexBufferGLES2::stride() const
{
    return mStride;
}

void RenderDeviceGLES2::VertexBufferGLES2::release()
{
    glDeleteBuffers(1, &mHandle);

    mDevice->mVertexBufferMemory -= mSize;
}

RenderDeviceGLES2::IndexBufferGLES2::IndexBufferGLES2(RenderDeviceGLES2* device) :
    mDevice(device)
{
    // Nothing else to do
}

RenderDeviceGLES2::IndexBufferGLES2::~IndexBufferGLES2()
{
    release();
}

bool RenderDeviceGLES2::IndexBufferGLES2::load(void* data, uint32_t size, Format format)
{
    release();

    glGenBuffers(1, &mHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    mDevice->mIndexBufferMemory += size;
    mSize = size;
    mFormat = format;
    return true;
}

bool RenderDeviceGLES2::IndexBufferGLES2::update(void* data, uint32_t size, uint32_t offset)
{
    if (offset + size > mSize) return false;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mHandle);
    if (offset == 0u && size == mSize) {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }
    else {
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
    }

    return true;
}

uint32_t RenderDeviceGLES2::IndexBufferGLES2::size() const
{
    return mSize;
}

IndexBuffer::Format RenderDeviceGLES2::IndexBufferGLES2::format() const
{
    return mFormat;
}

void RenderDeviceGLES2::IndexBufferGLES2::release()
{
    glDeleteBuffers(1, &mHandle);

    mDevice->mIndexBufferMemory -= mSize;
}

#endif
