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
thread_local GLenum toTexType[]      = {GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP};
thread_local GLenum toTexBinding[]   = {GL_TEXTURE_BINDING_2D, GL_TEXTURE_BINDING_CUBE_MAP};
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

    int maxTexUnits = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTexUnits);
    mMaxTextureUnits = std::min(16, maxTexUnits);

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

    for (uint8_t i = 0; i < mMaxTextureUnits; ++i) mTexSlots[i].texture = nullptr;

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

        // Bind textures and update sampler state
        if (mask & Textures) {
            for (uint8_t i = 0; i < mMaxTextureUnits; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                auto& slot = mTexSlots[i];

                if (!slot.texture) {
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    glBindTexture(GL_TEXTURE_3D_OES, 0);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
                else {
                    glBindTexture(slot.texture->mGlType, slot.texture->mHandle);
                    slot.texture->applyState();
                    slot.state = slot.texture->mState;
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
    if (mCurRenderBuffer) {
        if ((flags & ClrDepth) && mCurRenderBuffer->mDepthTex == 0) flags &= ~ClrDepth;
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
    // glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mDefaultFBO);

    // mCurIndexBuffer = reinterpret_cast<IndexBuffer*>(1u); mNewIndexBuffer = nullptr;
    // mCurVertexLayout = 1;                                 mNewVertexLayout = 0;
    mCurRasterState.hash = 0xFFFFFFFFu;                   mNewRasterState.hash = 0u;
    mCurBlendState.hash = 0xFFFFFFFFu;                    mNewBlendState.hash = 0u;
    mCurDepthStencilState.hash = 0xFFFFFFFFu;             mCurDepthStencilState.hash = 0u;

    setColorWriteMask(true);
    // mVertexBufUpdated = true;
    // commitStates();

    // Bind buffers
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);
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

Texture* RenderDeviceGLES2::newTexture()
{
    return new TextureGLES2(this);
}

void RenderDeviceGLES2::bind(const Texture* texture, uint8_t slot)
{
    auto tex = static_cast<const TextureGLES2*>(texture);

    if (mTexSlots[slot].texture != tex) {
        mTexSlots[slot].texture = tex;
        mPendingMask |= Textures;
    }
}

uint32_t RenderDeviceGLES2::usedTextureMemory() const
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

RenderBuffer* RenderDeviceGLES2::newRenderBuffer()
{
    return new RenderBufferGLES2(this);
}

void RenderDeviceGLES2::bind(RenderBuffer* buffer)
{
    auto rb = static_cast<RenderBufferGLES2*>(buffer);

    // Resolve the renderbuffer if necessary
    if (mCurRenderBuffer) mCurRenderBuffer->resolve();

    // Set the current renderbuffer
    mCurRenderBuffer = rb;

    if (!buffer) {
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
        for (uint8_t i = 0; i < mMaxTextureUnits; ++i) mTexSlots[i].texture = nullptr;
        commitStates(Textures);

        glBindFramebuffer(GL_FRAMEBUFFER, rb->mFboMS != 0 ? rb->mFboMS : rb->mFbo);

        mFbWidth  = rb->width();
        mFbHeight = rb->height();
    }
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

RenderDeviceGLES2::RenderBufferGLES2::RenderBufferGLES2(RenderDeviceGLES2* device) :
    mDevice(device)
{
    for (uint32_t i = 0; i < MaxColorAttachmentCount; ++i) {
        mColBufs[i] = 0u;
    }
}

RenderDeviceGLES2::RenderBufferGLES2::~RenderBufferGLES2()
{
    release();
}

bool RenderDeviceGLES2::RenderBufferGLES2::create(Texture::Format format, uint16_t width,
    uint16_t height, bool depth, uint8_t colBufCount, uint8_t samples)
{
    if (width == 0 || height == 0u) {
        Log::error("Failed to create render buffer: Invalid size (%ux%u)", width, height);
        return false;
    }

    if (
        (format == Texture::RGBA16F || format == Texture::RGBA32F)
        && !mDevice->mTexFloatSupported
    ) {
        Log::error("Failed to create render buffer: Float formats are unsupported");
        return false;
    }

    if (colBufCount > static_cast<unsigned int>(mDevice->mMaxColBuffers)) {
        Log::error("Failed to create render buffer: attempting to create buffer with %u color"
            " buffers. Maximum supported: %i", colBufCount, mDevice->mMaxColBuffers);
        return false;
    }

    uint32_t maxSamples = 0;
    if (glExt::EXT_multisampled_render_to_texture || glExt::ANGLE_framebuffer_multisample) {
        GLint value;
        glGetIntegerv(
            glExt::EXT_multisampled_render_to_texture ? GL_MAX_SAMPLES_EXT : GL_MAX_SAMPLES_ANGLE,
            &value
        );
        maxSamples = static_cast<uint32_t>(value);
    }
    if (samples > maxSamples) {
        samples = maxSamples;
        Log::warning(
            "GPU does not support desired level of multisampling quality for render buffer"
        );
    }

    mWidth = width;
    mHeight = height;
    mSamples = samples;
    mFormat = format;

    // Create framebuffers
    glGenFramebuffers(1, &mFbo);
    if (samples > 0) glGenFramebuffers(1, &mFboMS);

    if (colBufCount > 0) {
        // Attach color buffers
        for (uint8_t i = 0; i < colBufCount; ++i) {
            glBindFramebuffer(GL_FRAMEBUFFER, mFbo);


            // Create a color texture
            auto tex = static_cast<TextureGLES2*>(mDevice->newTexture());
            tex->create(Texture::_2D, format, width, height, false, false, false);
            tex->setData(nullptr, 0, 0);
            tex->mRenderBuffer = this;
            mColTexs[i] = std::shared_ptr<TextureGLES2>(tex);

            // Attach the texture
            if (samples > 0 && glExt::EXT_multisampled_render_to_texture) {
                glFramebufferTexture2DMultisampleEXT(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                    GL_TEXTURE_2D, tex->mHandle, 0, samples
                );
            }
            else {
                glFramebufferTexture2D(
                    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex->mHandle, 0
                );
            }

            if (samples > 0 && glExt::ANGLE_framebuffer_multisample) {
                if (format == Texture::RGBA8 && glExt::OES_rgb8_rgba8) {
                    glBindFramebuffer(GL_FRAMEBUFFER, mFboMS);

                    // Create a multisampled renderbuffer
                    glGenRenderbuffers(1, &mColBufs[i]);
                    glBindRenderbuffer(GL_RENDERBUFFER, mColBufs[i]);
                    glRenderbufferStorageMultisampleANGLE(
                        GL_RENDERBUFFER, samples, GL_RGBA8_OES, width, height
                    );

                    // Attach the renderbuffer
                    glFramebufferRenderbuffer(
                        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, mColBufs[i]
                    );
                }
                else {
                    Log::error("GPU does not support multisampling for this format");
                }
            }
        }
    }

    if (depth) {
        glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

        // Create depth texture
        if (
            (samples > 0 && glExt::EXT_multisampled_render_to_texture) ||
            (!glExt::OES_depth_texture && !glExt::ANGLE_depth_texture)
        ) {            
            glGenRenderbuffers(1, &mDepthBuf);
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuf);

            if (samples > 0 && glExt::EXT_multisampled_render_to_texture) {
                glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, samples,
                    mDevice->mDepthFormat, width, height);
            }
            else {
                glRenderbufferStorage(GL_RENDERBUFFER, mDevice->mDepthFormat, width, height);
            }

            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuf
            );
        }
        else {
            // Create a depth texture
            auto tex = static_cast<TextureGLES2*>(mDevice->newTexture());
            tex->create(Texture::_2D, Texture::DEPTH, width, height, false, false, false);
            if (glExt::EXT_shadow_samplers) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE);
            }
            tex->setData(nullptr, 0, 0);
            tex->mRenderBuffer = this;
            mDepthTex = std::shared_ptr<TextureGLES2>(tex);

            // Attach the texture
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->mHandle, 0
            );
        }

        if (samples > 0 && glExt::ANGLE_framebuffer_multisample) {
            glBindFramebuffer(GL_FRAMEBUFFER, mFboMS);

            // Create a multisampled renderbuffer
            glGenRenderbuffers(1, &mDepthBufMS);
            glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufMS);
            glRenderbufferStorageMultisampleANGLE(
                GL_RENDERBUFFER, samples, mDevice->mDepthFormat, width, height
            );

            // Attach the render buffer
            glFramebufferRenderbuffer(
                GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBufMS
            );
        }
    }

    // Check if FBO is complete
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    uint32_t status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool valid = (status == GL_FRAMEBUFFER_COMPLETE);

    if (valid && samples > 0 && glExt::ANGLE_framebuffer_multisample) {
        glBindFramebuffer(GL_FRAMEBUFFER, mFboMS);
        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        valid = (status == GL_FRAMEBUFFER_COMPLETE);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mDevice->mDefaultFBO);

    if (!valid) {
        release();
        return false;
    }

    return true;
}

Texture* RenderDeviceGLES2::RenderBufferGLES2::texture(uint8_t index)
{
    if (index < MaxColorAttachmentCount) {
        return mColTexs[index].get();
    }
    else if (index == 32) {
        return mDepthTex.get();
    }

    return nullptr;
}

uint16_t RenderDeviceGLES2::RenderBufferGLES2::width() const
{
    return mWidth;
}

uint16_t RenderDeviceGLES2::RenderBufferGLES2::height() const
{
    return mHeight;
}

Texture::Format RenderDeviceGLES2::RenderBufferGLES2::format() const
{
    return mFormat;
}

void RenderDeviceGLES2::RenderBufferGLES2::resolve()
{
    // Only needed when using ANGLE_framebuffer_multisample (which uses fboMS)
    if (!mFboMS) return;

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mFboMS);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mFbo);

    bool depthResolved {false};
    for (uint32_t i = 0; i < MaxColorAttachmentCount; ++i) {
        if (mColBufs[i] != 0) {
            int mask = GL_COLOR_BUFFER_BIT;
            if (
                !depthResolved && mDepthBufMS != 0 && mDepthTex != 0 &&
                !glExt::ANGLE_depth_texture
            ) {
                //Cannot resolve depth textures created with ANGLE_depth_texture
                mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
                depthResolved = true;
            }

            glBlitFramebufferANGLE(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, mask, GL_NEAREST);
        }
    }

    //Cannot resolve depth textures created with ANGLE_depth_texture
    if (!depthResolved && mDepthBufMS != 0 && mDepthTex != 0 && !glExt::ANGLE_depth_texture) {
        glBlitFramebufferANGLE(
            0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight,
            GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
        );
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER_ANGLE, mDevice->mDefaultFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER_ANGLE, mDevice->mDefaultFBO);
}

void RenderDeviceGLES2::RenderBufferGLES2::release()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mDevice->mDefaultFBO);

    for (uint32_t i = 0; i < MaxColorAttachmentCount; ++i) {
        if (mColBufs[i]) glDeleteRenderbuffers(1, &mColBufs[i]);
        mColTexs[i] = nullptr;
    }

    if (mDepthBuf) glDeleteRenderbuffers(1, &mDepthBuf);
    mDepthTex = nullptr;

    if (mFbo) glDeleteFramebuffers(1, &mFbo);
    if (mFboMS) glDeleteFramebuffers(1, &mFboMS);
}

RenderDeviceGLES2::TextureGLES2::TextureGLES2(RenderDeviceGLES2* device) :
    mDevice(device)
{
    // Nothing to do
}

RenderDeviceGLES2::TextureGLES2::~TextureGLES2()
{
    release();
}

bool RenderDeviceGLES2::TextureGLES2::create(Type type, Format format, uint16_t width,
    uint16_t height, bool hasMips, bool mipMaps, bool srgb)
{
    if (mRenderBuffer) {
        Log::error("Attempt to create a new texture over renderbuffer texture");
        return false;
    }

    if (mHandle) release();

    if (
        !mDevice->mPVRTCISupported && (format == PVRTCI_2BPP || format == PVRTCI_A2BPP ||
        format == PVRTCI_4BPP || format == PVRTCI_A4BPP)
    ) {
        Log::error("PVRTCI texture formats are not supported by the GPU");
        return false;
    }
    else if (!mDevice->mTexETC1Supported && format == ETC1) {
        Log::error("ETC1 texture format is not supported by the GPU");
        return false;
    }
    else if (!mDevice->mDXTSupported && (format == DXT1 || format == DXT3 || format == DXT5)) {
        Log::error("S3TC/DXT texture formats are not supported by the GPU. "
            "(Forgot to check for mobile?)");
        return false;
    }

    if (width == 0 || height == 0) {
        Log::error("Unable to create new texture: invalide size (%ux%u)", width, height);
        return false;
    }
    else if (!mDevice->mTexNPOTSupported && ((width & (width-1)) || (height & (height-1)))) {
        Log::error("Unable to create new texture: non-power-of-two textures are not supported by "
            "GPU");
        return false;
    }
    else if (
        type == Cube &&
        (width > mDevice->mMaxCubeTextureSize || height > mDevice->mMaxCubeTextureSize)
    ) {
        Log::error("Unable to create new texture: cube map size (%ux%u) is bigger than maximum "
            "(%ux%u)", width, height, mDevice->mMaxCubeTextureSize, mDevice->mMaxCubeTextureSize);
        return false;
    }
    else if (width > maxSize() || height > maxSize()) {
        Log::error("Unable to create new texture: texture size (%ux%u) is bigger than maximum "
            "(%ux%u)", width, height, maxSize(), maxSize());
        return false;
    }

    switch(format) {
    case RGBA8:
    case RGBA16F:
    case RGBA32F:
        mGlFormat = GL_RGBA;
        break;
    case DXT1:
        mGlFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case DXT3:
        mGlFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case DXT5:
        mGlFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    case PVRTCI_2BPP:
        mGlFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        break;
    case PVRTCI_A2BPP:
        mGlFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        break;
    case PVRTCI_4BPP:
        mGlFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        break;
    case PVRTCI_A4BPP:
        mGlFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        break;
    case ETC1:
        mGlFormat = GL_ETC1_RGB8_OES;
        break;
    case DEPTH:
        mGlFormat = GL_DEPTH_COMPONENT;
        break;
    default:
        Log::error("Unable to create new texture: invalid format");
        return false;
    }

    glGenTextures(1, &mHandle);
    if (mHandle == 0) {
        Log::error("Unable to create new texture: could not generate GPU texture");
        return false;
    }

    mGlType = toTexType[type];
    mType = type;
    mFormat = format;
    mWidth = width;
    mHeight = height;
    mSrgb = srgb;
    mHasMips = hasMips;
    mMipMaps = mipMaps;

    glActiveTexture(GL_TEXTURE0);

    // TODO: Add texture mutex lock here
    int lastTexture;
    glGetIntegerv(toTexBinding[mType], &lastTexture);

    glBindTexture(mGlType, mHandle);

    mState = 0;
    applyState();

    glBindTexture(mGlType, lastTexture);

    // Calculate memory requirements
    mMemSize = calcSize(format, width, height);
    if (hasMips || mipMaps) mMemSize += static_cast<int>(mMemSize / 3.f + 0.5f);
    if (type == Cube) mMemSize *= 6;
    mDevice->mTextureMemory += mMemSize;

    return true;
}

void RenderDeviceGLES2::TextureGLES2::setData(const void* buffer, uint8_t slice, uint8_t level)
{
    if (mRenderBuffer) {
        Log::error("Attempt to alter data of a render buffer texture");
        return;
    }

    glActiveTexture(GL_TEXTURE0);

    int lastTexture;
    glGetIntegerv(toTexBinding[mType], &lastTexture);

    glBindTexture(mGlType, mHandle);

    int inputFormat = GL_RGBA;
    int inputType = GL_UNSIGNED_BYTE;
    bool compressed = false;

    switch (mFormat) {
        case RGBA16F:
        case RGBA32F:
            inputType = GL_FLOAT;
            break;
        case DEPTH:
            inputFormat = GL_DEPTH_COMPONENT;
            inputType = GL_UNSIGNED_SHORT;
            break;
        case DXT1:
        case DXT3:
        case DXT5:
        case ETC1:
        case PVRTCI_2BPP:
        case PVRTCI_A2BPP:
        case PVRTCI_4BPP:
        case PVRTCI_A4BPP:
            compressed = true;
        default:
            break;
    }

    // Calculate size of the next mipmap using "floor" convention
    uint16_t w = std::max(mWidth >> level, 1);
    uint16_t h = std::max(mHeight >> level, 1);

    int target = (mType == _2D) ? GL_TEXTURE_2D : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice);

    if (compressed) {
        glCompressedTexImage2D(
            target, level, mGlFormat, w, h, 0, calcSize(mFormat, w, h), buffer
        );
    }
    else {
        glTexImage2D(
            target, level, mGlFormat, w, h, 0, inputFormat, inputType, buffer
        );
    }

    if (mHasMips && (mType == _2D || slice == 5)) {
        // Note: cube map mips are only generated when the last side is uploaded
        glGenerateMipmap(mGlType);
    }
    
    glBindTexture(mGlType, lastTexture);
}

void RenderDeviceGLES2::TextureGLES2::setSubData(const void* buffer, uint16_t x, uint16_t y,
    uint16_t width, uint16_t height, uint8_t slice, uint8_t level)
{
    if (mRenderBuffer) {
        Log::error("Attemting to alter data of a render buffer texture");
        return;
    }

    // Calculate size of the next mipmap using "floor" convention
    int w = std::max(mWidth >> level, 1);
    int h = std::max(mHeight >> level, 1);

    if (x + width > w || y + height > h) {
        Log::error("Attempting to update portion out of texture boundaries");
        return;
    }

    glActiveTexture(GL_TEXTURE0);

    int lastTexture;
    glGetIntegerv(toTexBinding[mType], &lastTexture);

    glBindTexture(mGlType, mHandle);

    int inputFormat = GL_RGBA;
    int inputType = GL_UNSIGNED_BYTE;
    bool compressed = false;

    switch (mFormat) {
        case RGBA16F:
        case RGBA32F:
            inputType = GL_FLOAT;
            break;
        case DEPTH:
            inputFormat = GL_DEPTH_COMPONENT;
            inputType = GL_UNSIGNED_SHORT;
            break;
        case DXT1:
        case DXT3:
        case DXT5:
        case ETC1:
        case PVRTCI_2BPP:
        case PVRTCI_A2BPP:
        case PVRTCI_4BPP:
        case PVRTCI_A4BPP:
            compressed = true;
        default:
            break;
    }

    int target = (mType == _2D) ? GL_TEXTURE_2D : (GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice);

    if (compressed) {
        glCompressedTexSubImage2D(
            target, level, x, y, width, height, mGlFormat, calcSize(mFormat, width, height), buffer
        );
    }
    else {
        glTexSubImage2D(target, level, x, y, width, height, inputFormat, inputType, buffer);
    }

    if (mHasMips && (mType == _2D || slice == 5)) {
        // Note: cube map mips are only generated when the last side is uploaded
        glGenerateMipmap(mGlType);
    }
    
    glBindTexture(mGlType, lastTexture);
}

bool RenderDeviceGLES2::TextureGLES2::data(void* buffer, uint8_t slice, uint8_t level) const
{
    // Can only retrieve RGBA8 data
    if (mGlFormat != RGBA8) return false;

    int target = (mType == _2D) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;

    GLuint fb {0u};
    glGenFramebuffers(1, &fb);
    if (!fb) return false;

    GLint prevFb;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFb);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, mHandle, level);
    glReadPixels(0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    glDeleteFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, prevFb);
    return true;
}

uint32_t RenderDeviceGLES2::TextureGLES2::bufferSize() const
{
    return calcSize(mFormat, mWidth, mHeight);
}

uint16_t RenderDeviceGLES2::TextureGLES2::width() const
{
    return mWidth;
}

uint16_t RenderDeviceGLES2::TextureGLES2::height() const
{
    return mHeight;
}

void RenderDeviceGLES2::TextureGLES2::setFilter(Filter filter)
{
    if ((mState & _FilterMask) != filter) {
        mState &= ~_FilterMask;
        mState |= filter;

        mDevice->mPendingMask |= Textures;
    }
}

void RenderDeviceGLES2::TextureGLES2::setAnisotropyLevel(Anisotropy aniso)
{
    if ((mState & _AnisotropyMask) != aniso) {
        mState &= ~_AnisotropyMask;
        mState |= aniso;

        mDevice->mPendingMask |= Textures;
    }
}

void RenderDeviceGLES2::TextureGLES2::setRepeating(uint32_t repeating)
{
    if ((mState & _RepeatingMask) != repeating) {
        mState &= ~_RepeatingMask;
        mState |= repeating;

        mDevice->mPendingMask |= Textures;
    }
}

void RenderDeviceGLES2::TextureGLES2::setLessOrEqual(bool enabled)
{
    if (enabled && !(mState & LEqual)) {
        mState |= LEqual;
        mDevice->mPendingMask |= Textures;
    }
    else if (!enabled && (mState & LEqual)) {
        mState &= ~LEqual;
        mDevice->mPendingMask |= Textures;
    }
}

Texture::Filter RenderDeviceGLES2::TextureGLES2::filter() const
{
    return static_cast<Filter>(mState & _FilterMask);
}

Texture::Anisotropy RenderDeviceGLES2::TextureGLES2::anisotropyLevel() const
{
    return static_cast<Anisotropy>(mState & _AnisotropyMask);
}

uint32_t RenderDeviceGLES2::TextureGLES2::repeating() const
{
    return mState & _RepeatingMask;
}

bool RenderDeviceGLES2::TextureGLES2::lessOrEqual() const
{
    return mState & LEqual;
}

bool RenderDeviceGLES2::TextureGLES2::flipCoords() const
{
    return mRenderBuffer != nullptr;
}

Texture::Type RenderDeviceGLES2::TextureGLES2::type() const
{
    return mType;
}

Texture::Format RenderDeviceGLES2::TextureGLES2::format() const
{
    return mFormat;
}

void RenderDeviceGLES2::TextureGLES2::release()
{
    glDeleteTextures(1, &mHandle);
    mDevice->mTextureMemory -= mMemSize;
}

void RenderDeviceGLES2::TextureGLES2::applyState() const
{
    thread_local const uint32_t maxAniso[] = {1u, 2u, 4u, 8u, 16u, 1u, 1u, 1u};
    thread_local const uint32_t wrapModes[] = {GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_EDGE};
    thread_local const uint32_t magFilters[] = {GL_LINEAR, GL_LINEAR, GL_NEAREST};
    thread_local const uint32_t minFiltersMips[] = {
        GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST
    };

    auto which = (mState & _FilterMask) >> _FilterStart;
    glTexParameteri(
        mGlType, GL_TEXTURE_MIN_FILTER, mHasMips ? minFiltersMips[which] : magFilters[which]
    );

    which = (mState & _FilterMask) >> _FilterStart;
    glTexParameteri(mGlType, GL_TEXTURE_MAG_FILTER, magFilters[which]);

    if (glExt::EXT_texture_filter_anisotropic) {
        which = (mState & _AnisotropyMask) >> _AnisotropyStart;
        glTexParameteri(mGlType, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso[which]);
    }

    which = (mState & _RepeatingMaskX) >> _RepeatingStartX;
    glTexParameteri(mGlType, GL_TEXTURE_WRAP_S, wrapModes[which]);

    which = (mState & _RepeatingMaskY) >> _RepeatingStartY;
    glTexParameteri(mGlType, GL_TEXTURE_WRAP_T, wrapModes[which]);

    if (mDevice->mTexShadowSamplers) {
        if (mState & LEqual) {
            glTexParameteri(
                GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_COMPARE_REF_TO_TEXTURE_EXT
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_EXT, GL_LEQUAL);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_EXT, GL_NONE);
        }
    }
}

#endif
