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

#include "renderdevicegl.hpp"

#if !defined(NX_OPENGL_ES)

#include "../system/log.hpp"
#include "opengl.hpp"

#include <mutex>
#include <algorithm>

// Locals
thread_local const char* defaultShaderVS =
    "uniform mat4 viewProjMat;\n"
    "uniform mat4 worldMat;\n"
    "attribute vec3 vertPos;\n"
    "void main() {\n"
    "   gl_Position = viewProjMat * worldMat * vec4(vertPos, 1.0);\n"
    "}\n";

thread_local const char* defaultShaderFS =
    "uniform vec4 color;\n"
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
    int temp;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &mMaxCubeTextureSize);

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &temp);
    mMaxTextureUnits = std::min(16, temp);

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &temp);
    mMaxColBuffers = std::min(4, temp);

    mDXTSupported = true;
    mPVRTCISupported = false;
    mTexETC1Supported = false;

    mTexFloatSupported = glExt::ARB_texture_float;
    mTexDepthSupported = true;
    mTexShadowSamplers = true;

    mTex3DSupported = true;
    mTexNPOTSupported = glExt::ARB_texture_non_power_of_two;
    mTexSRGBSupported = true;

    mRTMultiSampling = glExt::EXT_framebuffer_multisample;

    mOccQuerySupported = true;
    mTimerQuerySupported = true;

    // Set some default values
    mIndexFormat = GL_UNSIGNED_SHORT;
    mActiveVertexAttribsMask = 0u;

    // Find supported depth format (some old ATI cards only support 16 bit depth for FBOs)
    mDepthFormat = GL_DEPTH_COMPONENT24;

    RenderBuffer* buffer = newRenderBuffer();
    if (!buffer->create(Texture::RGBA8, 32, 32, true, 0, 0)) {
        mDepthFormat = GL_DEPTH_COMPONENT16;
        Log::warning("Render target depth precision limited to 16 bits");
    }
    delete buffer;

    initStates();
    resetStates();

    return true;
}

void RenderDeviceGL::initStates()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void RenderDeviceGL::resetStates()
{
    mCurIndexBuffer = reinterpret_cast<IndexBuffer*>(1u); mNewIndexBuffer = nullptr;
    mCurVertexLayout = 1;                                 mNewVertexLayout = 0;
    mCurRasterState.hash = 0xFFFFFFFFu;                   mNewRasterState.hash = 0u;
    mCurBlendState.hash = 0xFFFFFFFFu;                    mNewBlendState.hash = 0u;
    mCurDepthStencilState.hash = 0xFFFFFFFFu;             mCurDepthStencilState.hash = 0u;

    for (uint8_t i = 0; i < 16u; ++i) mTexSlots[i].texture = nullptr;

    setColorWriteMask(true);
    mPendingMask = 0xFFFFFFFFu;
    mVertexBufUpdated = true;
    commitStates();

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
}

bool RenderDeviceGL::commitStates(uint32_t filter)
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
                    mNewIndexBuffer ? static_cast<IndexBufferGL*>(mNewIndexBuffer)->mHandle : 0
                );

                mCurIndexBuffer = mNewIndexBuffer;
            }

            mPendingMask &= ~IndexBuffers;
        }

        // Bind textures and update sampler state
        if (mask & (Textures | SamplerState)) {
            for (uint8_t i = 0; i < 16u; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                auto& slot = mTexSlots[i];

                if (mask & Textures) {
                    if (!slot.texture) {
                        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                        glBindTexture(GL_TEXTURE_2D, 0);
                    }
                    else {
                        glBindTexture(slot.texture->mGlType, slot.texture->mHandle);
                    }
                }

                // Apply sampler state
                if (slot.texture && (mask & SamplerState || slot.texture->mState != slot.state)) {
                    slot.texture->applyState();
                    slot.state = slot.texture->mState;
                }
            }

            mPendingMask &= ~(Textures | SamplerState);
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

void RenderDeviceGL::clear(uint32_t flags, const float* color, float depth)
{
    uint32_t prevBuffers[4] = {0};

    if (mCurRenderBuffer != 0) {

        if ((flags & ClrDepth) && mCurRenderBuffer->mDepthTex == 0) flags &= ~ClrDepth;

        for (uint8_t i = 0; i < 4u; ++i) {
            glGetIntegerv(GL_DRAW_BUFFER0 + i, reinterpret_cast<int*>(&prevBuffers[i]));
        }

        uint32_t buffers[4], cnt = 0;
        if (flags & ClrColorRT0 && mCurRenderBuffer->mColTexs[0]) {
            buffers[cnt++] = GL_COLOR_ATTACHMENT0_EXT;
        }
        if (flags & ClrColorRT1 && mCurRenderBuffer->mColTexs[1]) {
            buffers[cnt++] = GL_COLOR_ATTACHMENT1_EXT;
        }
        if (flags & ClrColorRT2 && mCurRenderBuffer->mColTexs[2]) {
            buffers[cnt++] = GL_COLOR_ATTACHMENT2_EXT;
        }
        if (flags & ClrColorRT3 && mCurRenderBuffer->mColTexs[3]) {
            buffers[cnt++] = GL_COLOR_ATTACHMENT3_EXT;
        }

        if (cnt == 0) {
            flags &= ~(ClrColorRT0 | ClrColorRT1 | ClrColorRT2 | ClrColorRT3);
        }
        else {
            glDrawBuffers(cnt, buffers);
        }
    }

    uint32_t oglClearMask = 0;

    if (flags & ClrDepth) {
        oglClearMask |= GL_DEPTH_BUFFER_BIT;
        glClearDepth(depth);
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

    if (mCurRenderBuffer) glDrawBuffers(4, prevBuffers);
}

void RenderDeviceGL::draw(PrimType primType, uint32_t firstVert, uint32_t vertCount)
{
    if (commitStates()) {
        glDrawArrays(toPrimType[primType], firstVert, vertCount);
    }
}

void RenderDeviceGL::drawIndexed(PrimType primType, uint32_t firstIndex, uint32_t indexCount)
{
    if (commitStates()) {
        firstIndex *= (mIndexFormat == GL_UNSIGNED_SHORT) ? sizeof(short) : sizeof(int);

        glDrawElements(toPrimType[primType], indexCount, mIndexFormat, (char*)0 + firstIndex);
    }
}

void RenderDeviceGL::beginRendering()
{
    // Get the currently bound frame buffer object.
    // glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &mDefaultFBO);

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
    // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
}

void RenderDeviceGL::finishRendering()
{
    // Nothing to do
}

uint32_t RenderDeviceGL::registerVertexLayout(uint8_t numAttribs,
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

Texture* RenderDeviceGL::newTexture()
{
    return new TextureGL(this);
}

void RenderDeviceGL::bind(const Texture* texture, uint8_t slot)
{
    auto tex = static_cast<const TextureGL*>(texture);

    if (mTexSlots[slot].texture != tex) {
        mTexSlots[slot].texture = tex;
        mPendingMask |= Textures;
    }
}

uint32_t RenderDeviceGL::usedTextureMemory() const
{
    return mTextureMemory;
}

Shader* RenderDeviceGL::newShader()
{
    return new ShaderGL(this);
}

void RenderDeviceGL::bind(Shader* shader)
{
    glUseProgram(shader ? static_cast<ShaderGL*>(shader)->mHandle : 0);

    mCurShader = shader;
    mPendingMask |= VertexLayouts;
}

const std::string& RenderDeviceGL::getShaderLog()
{
    return shaderLog;
}

const char* RenderDeviceGL::getDefaultVSCode()
{
    return defaultShaderVS;
}

const char* RenderDeviceGL::getDefaultFSCode()
{
    return defaultShaderFS;
}

Shader* RenderDeviceGL::getCurrentShader() const
{
    return mCurShader;
}

VertexBuffer* RenderDeviceGL::newVertexBuffer()
{
    return new VertexBufferGL(this);
}

uint32_t RenderDeviceGL::usedVertexBufferMemory() const
{
    return mVertexBufferMemory;
}

void RenderDeviceGL::bind(VertexBuffer* buffer, uint8_t slot, uint32_t offset)
{
    auto& vbSlot = mVertBufSlots[slot];
    if (vbSlot.vbObj != buffer || vbSlot.offset != offset) {
        vbSlot = {buffer, offset};
        mVertexBufUpdated = true;
        mPendingMask |= VertexLayouts;
    }
}

IndexBuffer* RenderDeviceGL::newIndexBuffer()
{
    return new IndexBufferGL(this);
}

uint32_t RenderDeviceGL::usedIndexBufferMemory() const
{
    return mIndexBufferMemory;
}

void RenderDeviceGL::bind(IndexBuffer* buffer)
{
    mIndexFormat = toIndexFormat[buffer ? buffer->format() : 0];
    mNewIndexBuffer = buffer;
    mPendingMask |= IndexBuffers;
}

RenderBuffer* RenderDeviceGL::newRenderBuffer()
{
    return new RenderBufferGL(this);
}

void RenderDeviceGL::bind(RenderBuffer* buffer)
{
    auto rb = static_cast<RenderBufferGL*>(buffer);

    // Resolve the renderbuffer if necessary
    if (mCurRenderBuffer) mCurRenderBuffer->resolve();

    // Set the current renderbuffer
    mCurRenderBuffer = rb;

    if (!buffer) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
        if (mDefaultFBO == 0) glDrawBuffer(mOutputBufferIndex ? GL_BACK_RIGHT : GL_BACK_LEFT);

        mFbWidth  = mVpX + mVpWidth;
        mFbHeight = mVpY + mVpHeight;
        glDisable(GL_MULTISAMPLE);
    }
    else {
        // Unbind all textures to make sure that no FBO is bound anymore
        for (uint8_t i = 0; i < 16u; ++i) mTexSlots[i].texture = nullptr;
        commitStates(Textures);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb->mFboMS ? rb->mFboMS : rb->mFbo);

        mFbWidth  = rb->width();
        mFbHeight = rb->height();

        if (rb->mFboMS) {
            glEnable(GL_MULTISAMPLE);
        }
        else {
            glDisable(GL_MULTISAMPLE);
        }
    }
}

void RenderDeviceGL::setViewport(int x, int y, int width, int height)
{
    mVpX      = x;
    mVpY      = y;
    mVpWidth  = width;
    mVpHeight = height;

    mPendingMask |= Viewport;
}

void RenderDeviceGL::setScissorRect(int x, int y, int width, int height)
{
    mScX      = x;
    mScY      = y;
    mScWidth  = width;
    mScHeight = height;

    mPendingMask |= Scissor;
}

void RenderDeviceGL::setVertexLayout(uint32_t vlObj)
{
    mNewVertexLayout = vlObj;
}

void RenderDeviceGL::setColorWriteMask(bool enabled)
{
    mNewRasterState.renderTargetWriteMask = enabled;
}

bool RenderDeviceGL::getColorWriteMask() const
{
    return mNewRasterState.renderTargetWriteMask;
}

void RenderDeviceGL::setFillMode(FillMode fillMode)
{
    mNewRasterState.fillMode = fillMode;
    mPendingMask |= RenderStates;
}

RenderDevice::FillMode RenderDeviceGL::getFillMode() const
{
    return static_cast<FillMode>(mNewRasterState.fillMode);
}

void RenderDeviceGL::setCullMode(CullMode cullMode)
{
    mNewRasterState.cullMode = cullMode;
    mPendingMask |= RenderStates;
}

RenderDevice::CullMode RenderDeviceGL::getCullMode() const
{
    return static_cast<CullMode>(mNewRasterState.cullMode);
}

void RenderDeviceGL::setScissorTest(bool enabled)
{
    mNewRasterState.scissorEnable = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGL::getScissorTest() const
{
    return mNewRasterState.scissorEnable;
}

void RenderDeviceGL::setMultisampling(bool enabled)
{
    mNewRasterState.multisampleEnable = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGL::getMultisampling() const
{
    return mNewRasterState.multisampleEnable;
}

void RenderDeviceGL::setAlphaToCoverage(bool enabled)
{
    mNewBlendState.alphaToCoverageEnable = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGL::getAlphaToCoverage() const
{
    return mNewBlendState.alphaToCoverageEnable;
}

void RenderDeviceGL::setBlendMode(bool enabled, BlendFunc src, BlendFunc dst)
{
    mNewBlendState.blendEnable = enabled;
    mNewBlendState.srcBlendFunc = src;
    mNewBlendState.dstBlendFunc = dst;

    mPendingMask |= RenderStates;
}

bool RenderDeviceGL::getBlendMode(BlendFunc& src, BlendFunc& dst) const
{
    src = static_cast<BlendFunc>(mNewBlendState.srcBlendFunc);
    dst = static_cast<BlendFunc>(mNewBlendState.dstBlendFunc);
    return mNewBlendState.blendEnable;
}

void RenderDeviceGL::setDepthMask(bool enabled)
{
    mNewDepthStencilState.depthWriteMask = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGL::getDepthMask() const
{
    return mNewDepthStencilState.depthWriteMask;
}

void RenderDeviceGL::setDepthTest(bool enabled)
{
    mNewDepthStencilState.depthEnable = enabled;
    mPendingMask |= RenderStates;
}

bool RenderDeviceGL::getDepthTest() const
{
    return mNewDepthStencilState.depthEnable;
}

void RenderDeviceGL::setDepthFunc(DepthFunc depthFunc)
{
    mNewDepthStencilState.depthFunc = depthFunc;
    mPendingMask |= RenderStates;
}

RenderDevice::DepthFunc RenderDeviceGL::getDepthFunc() const
{
    return static_cast<DepthFunc>(mNewDepthStencilState.depthFunc);
}

void RenderDeviceGL::sync()
{
    glFinish();
}

void RenderDeviceGL::getCapabilities(uint32_t* maxTexUnits, uint32_t* maxTexSize,
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
    if (multithreading) *multithreading = true;
}

bool RenderDeviceGL::applyVertexLayout()
{
    if (mNewVertexLayout != 0) {
        if (!mCurShader) return false;

        uint32_t newVertexAttribMask {0u};

        ShaderGL* shader            = static_cast<ShaderGL*>(mCurShader);
        RDIInputLayout& inputLayout = shader->mInputLayouts[mNewVertexLayout-1];

        if (!inputLayout.valid) return false;

        // Set vertex attrib pointers
        auto& vl = mVertexLayouts[mNewVertexLayout-1];
        for (uint32_t i = 0; i < vl.numAttribs; ++i) {
            int8_t attribIndex = inputLayout.attribIndices[i];
            if (attribIndex >= 0) {
                VertexLayoutAttrib& attrib = vl.attribs[i];
                const auto& vbSlot = mVertBufSlots[attrib.vbSlot];

                VertexBufferGL* buffer = static_cast<VertexBufferGL*>(vbSlot.vbObj);
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

void RenderDeviceGL::applyRenderStates()
{
    // Rasterizer state
    if (mNewRasterState.hash != mCurRasterState.hash) {
        glPolygonMode(GL_FRONT_AND_BACK, mNewRasterState.fillMode == Solid ? GL_FILL : GL_LINE);

        if (mNewRasterState.cullMode != None) {
            glEnable(GL_CULL_FACE);
            glCullFace(mNewRasterState.cullMode == Back ? GL_BACK : GL_FRONT);
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
        glDepthMask(mNewDepthStencilState.depthWriteMask ? GL_TRUE : GL_FALSE);

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

RenderDeviceGL::ShaderGL::ShaderGL(RenderDeviceGL* device) :
    mDevice(device)
{
    for (uint32_t i = 0; i < MaxNumVertexLayouts; ++i) {
        memset(&mInputLayouts[i], 0, sizeof(RDIInputLayout));
    }
}

RenderDeviceGL::ShaderGL::~ShaderGL()
{
    if (mHandle) glDeleteProgram(mHandle);
}

bool RenderDeviceGL::ShaderGL::load(const char* vertexShader, const char* fragmentShader)
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
                    mInputLayouts[i].attribIndices[k] = static_cast<int8_t>(loc);
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

void RenderDeviceGL::ShaderGL::setUniform(int location, uint8_t type, float* data,
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

void RenderDeviceGL::ShaderGL::setSampler(int location, uint8_t unit)
{
    glUniform1i(location, static_cast<int>(unit));
}

int RenderDeviceGL::ShaderGL::uniformLocation(const char* name) const
{
    return glGetUniformLocation(mHandle, name);
}

int RenderDeviceGL::ShaderGL::samplerLocation(const char* name) const
{
    return glGetUniformLocation(mHandle, name);
}

RenderDeviceGL::VertexBufferGL::VertexBufferGL(RenderDeviceGL* device) :
    mDevice(device)
{
    // Nothing else to do
}

RenderDeviceGL::VertexBufferGL::~VertexBufferGL()
{
    release();
}

bool RenderDeviceGL::VertexBufferGL::load(void* data, uint32_t size, uint32_t stride)
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

bool RenderDeviceGL::VertexBufferGL::update(void* data, uint32_t size, uint32_t offset)
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

uint32_t RenderDeviceGL::VertexBufferGL::size() const
{
    return mSize;
}

uint32_t RenderDeviceGL::VertexBufferGL::stride() const
{
    return mStride;
}

void RenderDeviceGL::VertexBufferGL::release()
{
    glDeleteBuffers(1, &mHandle);

    mDevice->mVertexBufferMemory -= mSize;
}

RenderDeviceGL::IndexBufferGL::IndexBufferGL(RenderDeviceGL* device) :
    mDevice(device)
{
    // Nothing else to do
}

RenderDeviceGL::IndexBufferGL::~IndexBufferGL()
{
    release();
}

bool RenderDeviceGL::IndexBufferGL::load(void* data, uint32_t size, Format format)
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

bool RenderDeviceGL::IndexBufferGL::update(void* data, uint32_t size, uint32_t offset)
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

uint32_t RenderDeviceGL::IndexBufferGL::size() const
{
    return mSize;
}

IndexBuffer::Format RenderDeviceGL::IndexBufferGL::format() const
{
    return mFormat;
}

void RenderDeviceGL::IndexBufferGL::release()
{
    glDeleteBuffers(1, &mHandle);

    mDevice->mIndexBufferMemory -= mSize;
}

RenderDeviceGL::RenderBufferGL::RenderBufferGL(RenderDeviceGL* device) :
    mDevice(device)
{
    for (uint32_t i = 0; i < MaxColorAttachmentCount; ++i) {
        mColBufs[i] = 0u;
    }    
}

RenderDeviceGL::RenderBufferGL::~RenderBufferGL()
{
    release();
}

bool RenderDeviceGL::RenderBufferGL::create(Texture::Format format, uint16_t width,
    uint16_t height, bool depth, uint8_t colBufCount, uint8_t samples)
{
    if (width == 0u || height == 0u) {
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

    uint8_t maxSamples = 0u;
    if (mDevice->mRTMultiSampling) {
        GLint value;
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &value);
        maxSamples = static_cast<uint8_t>(value);
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
    glGenFramebuffersEXT(1, &mFbo);
    if (samples > 0) glGenFramebuffersEXT(1, &mFboMS);

    if (colBufCount > 0) {
        // Attach color buffers
        for (uint8_t i = 0; i < colBufCount; ++i) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFbo);

            // Create a color texture
            auto tex = static_cast<TextureGL*>(mDevice->newTexture());
            tex->create(Texture::_2D, format, width, height, false, false, false);
            tex->setData(nullptr, 0, 0);
            tex->mRenderBuffer = this;
            mColTexs[i] = std::shared_ptr<TextureGL>(tex);

            // Attach the texture
            glFramebufferTexture2DEXT(
                GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, tex->mHandle, 0
            );

            if (samples > 0) {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboMS);

                // Create a multisampled renderbuffer
                glGenRenderbuffersEXT(1, &mColBufs[i]);
                glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mColBufs[i]);
                glRenderbufferStorageMultisampleEXT(
                    GL_RENDERBUFFER_EXT, samples, tex->mGlFormat, width, height
                );

                // Attach the renderbuffer
                glFramebufferRenderbufferEXT(
                    GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,
                    GL_RENDERBUFFER_EXT, mColBufs[i]
                );
            }
        }

        static uint32_t buffers[] = {
            GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT,
            GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT
        };

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFbo);
        glDrawBuffers(colBufCount, buffers);

        if (samples > 0) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboMS);
            glDrawBuffers(colBufCount, buffers);
        }
    }
    else {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFbo);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (samples > 0) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboMS);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
    }

    if (depth) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFbo);

        // Create a depth texture
        auto tex = static_cast<TextureGL*>(mDevice->newTexture());
        tex->create(Texture::_2D, Texture::DEPTH, width, height, false, false, false);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        tex->setData(nullptr, 0, 0);
        tex->mRenderBuffer = this;
        mDepthTex = std::shared_ptr<TextureGL>(tex);

        // Attach texture
        glFramebufferTexture2DEXT(
            GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, tex->mHandle, 0
        );

        if (samples > 0) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboMS);

            // Create a multisampled render buffer
            glGenRenderbuffersEXT(1, &mDepthBuf);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthBuf);
            glRenderbufferStorageMultisampleEXT(
                GL_RENDERBUFFER_EXT, samples, mDevice->mDepthFormat, width, height
            );

            // Attach render buffer
            glFramebufferRenderbufferEXT(
                GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mDepthBuf
            );
        }
    }

    // Check if FBO is complete
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFbo);
    uint32_t status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    bool valid = (status == GL_FRAMEBUFFER_COMPLETE_EXT);

    if (valid && samples > 0) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFboMS);
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

        valid = (status == GL_FRAMEBUFFER_COMPLETE_EXT);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDevice->mDefaultFBO);

    if (!valid) {
        release();
        return false;
    }

    return true;
}

Texture* RenderDeviceGL::RenderBufferGL::texture(uint8_t index)
{
    if (index < MaxColorAttachmentCount) {
        return mColTexs[index].get();
    }
    else if (index == 32) {
        return mDepthTex.get();
    }

    return nullptr;
}

uint16_t RenderDeviceGL::RenderBufferGL::width() const
{
    return mWidth;
}

uint16_t RenderDeviceGL::RenderBufferGL::height() const
{
    return mHeight;
}

Texture::Format RenderDeviceGL::RenderBufferGL::format() const
{
    return mFormat;
}

void RenderDeviceGL::RenderBufferGL::resolve()
{
    if (!mFboMS) return;

    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, mFboMS);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, mFbo);

    bool depthResolved {false};
    for (uint32_t i = 0; i < MaxColorAttachmentCount; ++i) {
        if (mColBufs[i] != 0) {
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i);

            int mask = GL_COLOR_BUFFER_BIT;
            if (!depthResolved && mDepthBuf != 0) {
                mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
                depthResolved = true;
            }

            glBlitFramebufferEXT(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, mask, GL_NEAREST);
        }
    }

    if (!depthResolved && mDepthBuf != 0) {
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_NONE);
        glBlitFramebufferEXT(
            0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight,
            GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
        );
    }

    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, mDevice->mDefaultFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, mDevice->mDefaultFBO);
}

void RenderDeviceGL::RenderBufferGL::release()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDevice->mDefaultFBO);

    for (uint32_t i = 0; i < MaxColorAttachmentCount; ++i) {
        if (mColBufs[i]) glDeleteRenderbuffersEXT(1, &mColBufs[i]);
        mColTexs[i] = nullptr;
    }

    if (mDepthBuf) glDeleteRenderbuffersEXT(1, &mDepthBuf);
    mDepthTex = nullptr;

    if (mFbo) glDeleteFramebuffersEXT(1, &mFbo);
    if (mFboMS) glDeleteFramebuffersEXT(1, &mFboMS);
}

RenderDeviceGL::TextureGL::TextureGL(RenderDeviceGL* device) :
    mDevice(device)
{
    // Nothing to do
}

RenderDeviceGL::TextureGL::~TextureGL()
{
    release();
}

bool RenderDeviceGL::TextureGL::create(Type type, Format format, uint16_t width, uint16_t height,
    bool hasMips, bool mipMaps, bool srgb)
{
    if (mRenderBuffer) {
        Log::error("Attempt to create a new texture over renderbuffer texture");
        return false;
    }

    if (mHandle) release();

    if (width == 0 || height == 0) {
        Log::error("Unable to create new texture: invalide size (%ux%u)", width, height);
        return false;
    }

    if (!mDevice->mTexNPOTSupported && ((width & (width-1)) || (height & (height-1)))) {
        Log::error("Unable to create new texture: non-power-of-two textures are not supported by "
            "GPU");
        return false;
    }

    if (
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

    if (
        format == PVRTCI_2BPP || format == PVRTCI_A2BPP || format == PVRTCI_4BPP ||
        format == PVRTCI_A4BPP || format == ETC1
    ) {
        Log::error("Unable to create new texture: PVRTCI and ETC1 texture formats are not supported"
            " by the GPU (forgot to check for mobile maybe?)");
        return false;
    }

    switch(format) {
    case RGBA8:
        mGlFormat = srgb ? GL_SRGB8_ALPHA8_EXT : GL_RGBA8;
        break;
    case DXT1:
        mGlFormat = srgb ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case DXT3:
        mGlFormat = srgb ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case DXT5:
        mGlFormat = srgb ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    case RGBA16F:
        mGlFormat = GL_RGBA16F_ARB;
        break;
    case RGBA32F:
        mGlFormat = GL_RGBA32F_ARB;
        break;
    case DEPTH:
        mGlFormat = mDevice->mDepthFormat;
        break;
    default:
        Log::error("Unable to create new texture: invalid format");
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

    glGenTextures(1, &mHandle);
    if (mHandle == 0) {
        Log::error("Unable to create new texture: could not generate GPU texture");
        return false;
    }

    glActiveTexture(GL_TEXTURE15);

    // TODO: Add texture mutex lock here
    int lastTexture;
    glGetIntegerv(toTexBinding[mType], &lastTexture);

    glBindTexture(mGlType, mHandle);

    static float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

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

void RenderDeviceGL::TextureGL::setData(const void* buffer, uint8_t slice, uint8_t level)
{
    if (mRenderBuffer) {
        Log::error("Attempt to alter data of a render buffer texture");
        return;
    }

    glActiveTexture(GL_TEXTURE15);

    int lastTexture;
    glGetIntegerv(toTexBinding[mType], &lastTexture);

    glBindTexture(mGlType, mHandle);

    int inputFormat = GL_RGBA;
    int inputType = GL_UNSIGNED_BYTE;
    bool compressed = (mFormat == DXT1) || (mFormat == DXT3) || (mFormat == DXT5);

    switch (mFormat) {
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

    // Calculate size of the next mipmap using "floor" convention
    int w = std::max(mWidth >> level, 1);
    int h = std::max(mHeight >> level, 1);

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
        glEnable(mGlType);
        glGenerateMipmapEXT(mGlType);
        glDisable(mGlType);
    }
    
    glBindTexture(mGlType, lastTexture);
}

void RenderDeviceGL::TextureGL::setSubData(const void* buffer, uint16_t x, uint16_t y,
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

    glActiveTexture(GL_TEXTURE15);

    int lastTexture;
    glGetIntegerv(toTexBinding[mType], &lastTexture);

    glBindTexture(mGlType, mHandle);

    int inputFormat = GL_RGBA;
    int inputType = GL_UNSIGNED_BYTE;
    bool compressed = (mFormat == DXT1) || (mFormat == DXT3) || (mFormat == DXT5);

    switch (mFormat) {
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
        glEnable(mGlType);
        glGenerateMipmapEXT(mGlType);
        glDisable(mGlType);
    }
    
    glBindTexture(mGlType, lastTexture);
}

bool RenderDeviceGL::TextureGL::data(void* buffer, uint8_t slice, uint8_t level) const
{
    int target = (mType == _2D) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;

    int inputFormat = GL_RGBA;
    int inputType = GL_UNSIGNED_BYTE;
    bool compressed = false;

    switch (mFormat) {
    case RGBA8:
        break;
    case DXT1:
    case DXT3:
    case DXT5:
        compressed = true;
        break;  
    default:
        Log::error("Unable to get texture data: unsupported format");
        return false;
    }

    glActiveTexture(GL_TEXTURE15);

    int lastTexture;
    glGetIntegerv(toTexBinding[mType], &lastTexture);

    glBindTexture(mGlType, mHandle);

    if (compressed) {
        glGetCompressedTexImage(target, level, buffer);
    }
    else {
        glGetTexImage(target, level, inputFormat, inputType, buffer);
    }

    glBindTexture(mGlType, lastTexture);

    return true;
}

uint32_t RenderDeviceGL::TextureGL::bufferSize() const
{
    return calcSize(mFormat, mWidth, mHeight);
}

uint16_t RenderDeviceGL::TextureGL::width() const
{
    return mWidth;
}

uint16_t RenderDeviceGL::TextureGL::height() const
{
    return mHeight;
}

void RenderDeviceGL::TextureGL::setFilter(Filter filter)
{
    if ((mState & _FilterMask) != filter) {
        mState &= ~_FilterMask;
        mState |= filter;

        mDevice->mPendingMask |= SamplerState;
    }
}

void RenderDeviceGL::TextureGL::setAnisotropyLevel(Anisotropy aniso)
{
    if ((mState & _AnisotropyMask) != aniso) {
        mState &= ~_AnisotropyMask;
        mState |= aniso;

        mDevice->mPendingMask |= SamplerState;
    }
}

void RenderDeviceGL::TextureGL::setRepeating(uint32_t repeating)
{
    if ((mState & _RepeatingMask) != repeating) {
        mState &= ~_RepeatingMask;
        mState |= repeating;

        mDevice->mPendingMask |= SamplerState;
    }
}

void RenderDeviceGL::TextureGL::setLessOrEqual(bool enabled)
{
    if (enabled && !(mState & LEqual)) {
        mState |= LEqual;
        mDevice->mPendingMask |= SamplerState;
    }
    else if (!enabled && (mState & LEqual)) {
        mState &= ~LEqual;
        mDevice->mPendingMask |= SamplerState;
    }
}

Texture::Filter RenderDeviceGL::TextureGL::filter() const
{
    return static_cast<Filter>(mState & _FilterMask);
}

Texture::Anisotropy RenderDeviceGL::TextureGL::anisotropyLevel() const
{
    return static_cast<Anisotropy>(mState & _AnisotropyMask);
}

uint32_t RenderDeviceGL::TextureGL::repeating() const
{
    return mState & _RepeatingMask;
}

bool RenderDeviceGL::TextureGL::lessOrEqual() const
{
    return (mState & LEqual) != 0;
}

bool RenderDeviceGL::TextureGL::flipCoords() const
{
    return mRenderBuffer != nullptr;
}

Texture::Type RenderDeviceGL::TextureGL::type() const
{
    return mType;
}

Texture::Format RenderDeviceGL::TextureGL::format() const
{
    return mFormat;
}

void RenderDeviceGL::TextureGL::release()
{
    glDeleteTextures(1, &mHandle);
    mDevice->mTextureMemory -= mMemSize;
}

void RenderDeviceGL::TextureGL::applyState() const
{
    thread_local const uint32_t maxAniso[] = {1u, 2u, 4u, 8u, 16u, 1u, 1u, 1u};
    thread_local const uint32_t wrapModes[] = {GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_BORDER};
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

    which = (mState & _AnisotropyMask) >> _AnisotropyStart;
    glTexParameteri(mGlType, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso[which]);

    which = (mState & _RepeatingMaskX) >> _RepeatingStartX;
    glTexParameteri(mGlType, GL_TEXTURE_WRAP_S, wrapModes[which]);

    which = (mState & _RepeatingMaskY) >> _RepeatingStartY;
    glTexParameteri(mGlType, GL_TEXTURE_WRAP_T, wrapModes[which]);

    if (mState & LEqual) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }
}

#endif
