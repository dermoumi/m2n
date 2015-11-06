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
#include "renderdevicegles2.hpp"

#if defined(NX_OPENGL_ES)
//------------------------------------------------------------------------------
#include "../system/log.hpp"
#include "opengles2.hpp"

#include <mutex>

//==========================================================
// Locals
//==========================================================
static const char* defaultShaderVS =
    "uniform mat4 viewProjMat;\n"
    "uniform mat4 worldMat;\n"
    "attribute vec3 vertPos;\n"
    "void main() {\n"
    "   gl_Position = viewProjMat * worldMat * vec4(vertPos, 1.0);\n"
    "}\n";

static const char* defaultShaderFS =
    "uniform mediump vec4 color;\n"
    "void main() {\n"
    "   gl_FragColor = color;\n"
    "}\n";

static uint32_t toIndexFormat[] = {GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};
static uint32_t toPrimType[]    = {GL_TRIANGLES, GL_TRIANGLE_STRIP};
static int      toTexType[]     = {GL_TEXTURE_2D, GL_TEXTURE_3D_OES, GL_TEXTURE_CUBE_MAP};

static std::mutex vlMutex; // Vertex layouts mutex
static std::mutex txMutex; // Texture operations mutex
static std::mutex cpMutex; // Capabilities mutex
thread_local std::string shaderLog;

//----------------------------------------------------------
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
        Log::error("Extension EXT_texture_filter_anisotropic not supported");
        failed = true;
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
    mCaps.texFloat        = false;
    mCaps.texNPOT         = false;
    mCaps.rtMultisampling = glExt::ANGLE_framebuffer_blit || glExt::EXT_multisampled_render_to_texture;
    if( glExt::EXT_multisampled_render_to_texture ) glExt::ANGLE_framebuffer_blit = false;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &mMaxCubeTextureSize);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &mMaxTextureUnits);

    // Set some default values
    mIndexFormat = GL_UNSIGNED_SHORT;
    mActiveVertexAttribsMask = 0u;

    // TODO: Find supported depth format (some old ATI cards only support 16 bit depth for FBOs)

    initStates();
    resetStates();

    return true;
}

//----------------------------------------------------------
void RenderDeviceGLES2::initStates()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

//----------------------------------------------------------
void RenderDeviceGLES2::resetStates()
{
    mCurVertexLayout = 1;                     mNewVertexLayout = 0;
    mCurIndexBuffer = 1;                      mNewIndexBuffer = 0;
    mCurRasterState.hash = 0xFFFFFFFFu;       mNewRasterState.hash = 0u;
    mCurBlendState.hash = 0xFFFFFFFFu;        mNewBlendState.hash = 0u;
    mCurDepthStencilState.hash = 0xFFFFFFFFu; mCurDepthStencilState.hash = 0u;

    for (uint32_t i = 0; i < 16; ++i) setTexture(i, 0, 0);

    setColorWriteMask(true);
    mPendingMask = 0xFFFFFFFFu;
    commitStates();

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, mDefaultFBO);
}

//----------------------------------------------------------
bool RenderDeviceGLES2::commitStates(uint32_t filter)
{
    uint32_t mask = mPendingMask & filter;
    if (mask) {
        // Set viewport
        if (mask & PMViewport) {
            glViewport(mVpX, mVpY, mVpWidth, mVpHeight);
            mPendingMask &= ~PMViewport;
        }

        // Update renderstates
        if (mask & PMRenderStates) {
            applyRenderStates();
            mPendingMask &= ~PMRenderStates;
        }

        // Set scissor rect
        if (mask & PMScissor)
        {
            glScissor(mScX, mScY, mScWidth, mScHeight);
            mPendingMask &= ~PMScissor;
        }

        // Bind index buffer
        if (mask & PMIndexBuffer) {
            if (mNewIndexBuffer != mCurIndexBuffer) {
                if (mNewIndexBuffer == 0) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                }
                else {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffers.getRef(mNewIndexBuffer).glObj);
                }

                mCurIndexBuffer = mNewIndexBuffer;
            }

            mPendingMask &= ~PMIndexBuffer;
        }

        // Bind textures and set sampler state
        if (mask & PMTextures) {
            for (uint32_t i = 0; i < mMaxTextureUnits; ++i) {
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
            
            mPendingMask &= ~PMTextures;
        }

        // Bind vertex buffers
        if (mask & PMVertexLayout) {
            if (!applyVertexLayout()) return false;

            mCurVertexLayout = mNewVertexLayout;
            mPrevShaderID    = mCurShaderID;
            mPendingMask &= ~PMVertexLayout;
        }
    }

    return true;
}

//----------------------------------------------------------
void RenderDeviceGLES2::clear(const float* color)
{
    commitStates(PMViewport | PMScissor | PMRenderStates);
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------
void RenderDeviceGLES2::draw(RDIPrimType primType, uint32_t firstVert, uint32_t vertCount)
{
    if (commitStates()) {
        glDrawArrays(toPrimType[primType], firstVert, vertCount);
    }
}

//----------------------------------------------------------
void RenderDeviceGLES2::drawIndexed(RDIPrimType primType, uint32_t firstIndex, uint32_t indexCount)
{
    if (commitStates()) {
        firstIndex *= (mIndexFormat == GL_UNSIGNED_SHORT) ? sizeof(short) : sizeof(int);

        glDrawElements(toPrimType[primType], indexCount, mIndexFormat, (char*)0 + firstIndex);
    }
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::registerVertexLayout(uint32_t numAttribs,
    const VertexLayoutAttrib* attribs)
{
    if (mNumVertexLayouts == MaxNumVertexLayouts) return 0;

    std::lock_guard<std::mutex> lock(vlMutex);
    
    mVertexLayouts[mNumVertexLayouts].numAttribs = numAttribs;
    for (uint32_t i = 0; i < numAttribs; ++i) {
        mVertexLayouts[mNumVertexLayouts].attribs[i] = attribs[i];
    }

    return ++mNumVertexLayouts;
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::createTexture(TextureType::Type type, int width, int height,
    unsigned int depth, TextureFormat::Type format, bool hasMips, bool genMips, bool sRGB)
{
    // TODO: Add support for android specific texture compression

    if (!mCaps.texNPOT && ((width & (width-1)) != 0 || ((height & (height-1)) != 0))) {
        Log::error("Non-Power-Of-Two textures are not supported by GPU");
        return 0;
    }

    if (type == TextureType::TexCube && (width > mMaxCubeTextureSize || height > mMaxTextureSize)) {
        Log::error("Cube map is bigger than limit size");
        return 0;
    }
    else if (type != TextureType::TexCube && (width > mMaxTextureSize || height > mMaxTextureSize ||
        depth > mMaxTextureSize))
    {
        Log::error("Texture is bigger than limit size");
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
    case TextureFormat::RGBA8:
        tex.glFmt = GL_RGBA;
        break;
    case TextureFormat::DXT1:
        tex.glFmt = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case TextureFormat::DXT3:
        tex.glFmt = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case TextureFormat::DXT5:
        tex.glFmt = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    case TextureFormat::RGBA16F:
        tex.glFmt = GL_RGBA;
        break;
    case TextureFormat::RGBA32F:
        tex.glFmt = GL_RGBA;
        break;
    case TextureFormat::DEPTH:
        tex.glFmt = GL_DEPTH_COMPONENT;
    default:
        Log::warning("Could not create texture: invalid format");
        return 0;
    }

    glGenTextures(1, &tex.glObj);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(tex.type, tex.glObj);

    tex.samplerState = 0;
    applySamplerState(tex);

    glBindTexture(tex.type, 0);
    {
        std::lock_guard<std::mutex> lock(txMutex);
        if (mTexSlots[0].texObj) {
            auto& tex0 = mTextures.getRef(mTexSlots[0].texObj);
            glBindTexture(tex0.type, tex0.glObj);
        }
    }

    // Calculate memory requirements
    tex.memSize = calcTextureSize(format, width, height, depth);
    if (hasMips || genMips) tex.memSize += static_cast<int>(tex.memSize / 3.f + 0.5f);
    if (type == TextureType::TexCube) tex.memSize *= 6;
    mTextureMemory += tex.memSize;

    return mTextures.add(tex);
}

//----------------------------------------------------------
void RenderDeviceGLES2::uploadTextureData(uint32_t texObj, int slice, int mipLevel, const void* pixels)
{
    // TODO: return false when failing

    const auto& tex = mTextures.getRef(texObj);
    auto format     = tex.format;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(tex.type, tex.glObj);

    int inputFormat = GL_RGBA;
    int inputType = GL_UNSIGNED_BYTE;
    bool compressed = (format == TextureFormat::DXT1) || (format == TextureFormat::DXT3) ||
                      (format == TextureFormat::DXT5);

    switch (format) {
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            inputFormat = GL_RGBA;
            inputType   = GL_FLOAT;
            break;
        case TextureFormat::DEPTH:
            inputFormat = GL_DEPTH_COMPONENT;
            inputType   = GL_FLOAT;
            break;
        default:
            break;
    }

    // Calculate size of the next mipmap using "floor" convention
    int width = std::max(tex.width >> mipLevel, 1);
    int height = std::max(tex.height >> mipLevel, 1);

    if (tex.type == GL_TEXTURE_2D || tex.type == GL_TEXTURE_CUBE_MAP)
    {
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

    glBindTexture(tex.type, 0);
    {
        std::lock_guard<std::mutex> lock(txMutex);
        if (mTexSlots[0].texObj) {
            auto& tex0 = mTextures.getRef(mTexSlots[0].texObj);
            glBindTexture(tex0.type, tex0.glObj);
        }
    }
}

//----------------------------------------------------------
void RenderDeviceGLES2::destroyTexture(uint32_t texObj)
{
    if (texObj == 0) return;
    const auto& tex = mTextures.getRef(texObj);

    glDeleteTextures(1, &tex.glObj);
    mTextureMemory -= tex.memSize;
    mTextures.remove(texObj);
}

//----------------------------------------------------------
bool RenderDeviceGLES2::getTextureData(uint32_t texObj, int slice, int mipLevel, void* buffer)
{
    // TODO: Implement drawing to a render buffer and reading from it
    return false;
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::getTextureMemory() const
{
    return mTextureMemory;
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::createShader(const char* vertexShaderSrc, const char* fragmentShaderSrc)
{
    // Compile and link shader
    uint32_t programObj = createShaderProgram(vertexShaderSrc, fragmentShaderSrc);
    if (programObj == 0) return 0;
    if (!linkShaderProgram(programObj)) return 0;

    RDIShader shader {};
    shader.oglProgramObj = programObj;

    int attribCount;
    glGetProgramiv(programObj, GL_ACTIVE_ATTRIBUTES, &attribCount);

    // Run through vertex layouts and check which is compatible with this shader
    for (uint32_t i = 0; i < mNumVertexLayouts; ++i) {
        bool allAttribsFound = true;

        // Reset attribute indices to -1 (no attribute)
        for (uint32_t j = 0; j < 16; ++j) {
            shader.inputLayouts[i].attribIndices[j] = -1;
        }

        // Check if shader has all declared attributes, and set locations
        for (int j = 0; j < attribCount; ++j)
        {
            char name[32];
            uint32_t size, type;
            glGetActiveAttrib(programObj, j, 32, nullptr, (int*)&size, &type, name);

            bool attribFound = false;

            {
                std::lock_guard<std::mutex> lock(vlMutex);

                RDIVertexLayout& vl = mVertexLayouts[i];
                for (uint32_t k = 0; k < vl.numAttribs; ++k) {
                    if (vl.attribs[k].semanticName == name) {
                        auto loc = glGetAttribLocation(programObj, name);
                        shader.inputLayouts[i].attribIndices[k] = loc;
                        attribFound = true;
                    }
                }
            }

            if (!attribFound) {
                allAttribsFound = false;
                break;
            }
        }

        // An input layout is only valid for this shader if all attributes were found
        shader.inputLayouts[i].valid = allAttribsFound;
    }

    return mShaders.add(shader, true);
}

//----------------------------------------------------------
void RenderDeviceGLES2::destroyShader(uint32_t shaderID)
{
    if (shaderID == 0) return;
    RDIShader &shader = mShaders.getRef(shaderID);
    if (shader.oglProgramObj != 0) glDeleteProgram(shader.oglProgramObj);
    mShaders.remove(shaderID);
}

//----------------------------------------------------------
void RenderDeviceGLES2::bindShader(uint32_t shaderID)
{
    if (shaderID == 0) {
        glUseProgram(0);
    }
    else {
        RDIShader& shader = mShaders.getRef(shaderID);
        glUseProgram(shader.oglProgramObj);
    }

    mCurShaderID = shaderID;
    mPendingMask |= PMVertexLayout;
}

//----------------------------------------------------------
const std::string& RenderDeviceGLES2::getShaderLog()
{
    return shaderLog;
}

//----------------------------------------------------------
int RenderDeviceGLES2::getShaderConstLoc(uint32_t shaderID, const char* name)
{
    RDIShader& shader = mShaders.getRef(shaderID);
    return glGetUniformLocation(shader.oglProgramObj, name);
}

//----------------------------------------------------------
int RenderDeviceGLES2::getShaderSamplerLoc(uint32_t shaderID, const char* name)
{
    RDIShader& shader = mShaders.getRef(shaderID);
    return glGetUniformLocation(shader.oglProgramObj, name);
}

//----------------------------------------------------------
void RenderDeviceGLES2::setShaderConst(int loc, RDIShaderConstType type, float* values, uint32_t count)
{
    switch(type) {
    case CONST_FLOAT:
        glUniform1fv(loc, count, values);
        break;
    case CONST_FLOAT2:
        glUniform2fv(loc, count, values);
        break;
    case CONST_FLOAT3:
        glUniform3fv(loc, count, values);
        break;
    case CONST_FLOAT4:
        glUniform4fv(loc, count, values);
        break;
    case CONST_FLOAT44:
        glUniformMatrix4fv(loc, count, false, values);
        break;
    case CONST_FLOAT33:
        glUniformMatrix4fv(loc, count, false, values);
        break;
    }
}

//----------------------------------------------------------
void RenderDeviceGLES2::setShaderSampler(int loc, uint32_t texUnit)
{
    glUniform1i(loc, (int)texUnit);
}

//----------------------------------------------------------
const char* RenderDeviceGLES2::getDefaultVSCode()
{   
    return defaultShaderVS;
}

//----------------------------------------------------------
const char* RenderDeviceGLES2::getDefaultFSCode()
{
    return defaultShaderFS;
}

//----------------------------------------------------------
void RenderDeviceGLES2::beginRendering()
{
    // Get the currently bound frame buffer object. 
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mDefaultFBO);

    resetStates();
}

//----------------------------------------------------------
void RenderDeviceGLES2::finishRendering()
{
    // Nothing to do
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::createVertexBuffer(uint32_t size, const void* data)
{
    RDIBuffer buf;

    buf.type = GL_ARRAY_BUFFER;
    buf.size = size;
    glGenBuffers(1, &buf.glObj);
    glBindBuffer(buf.type, buf.glObj);
    glBufferData(buf.type, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(buf.type, 0);

    mBufferMemory += size;
    return mBuffers.add(buf);
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::createIndexBuffer(uint32_t size, const void* data)
{
    RDIBuffer buf;

    buf.type = GL_ELEMENT_ARRAY_BUFFER;
    buf.size = size;
    glGenBuffers(1, &buf.glObj);
    glBindBuffer(buf.type, buf.glObj);
    glBufferData(buf.type, size, data, GL_DYNAMIC_DRAW);
    glBindBuffer(buf.type, 0);

    mBufferMemory += size;
    return mBuffers.add(buf);
}

//----------------------------------------------------------
void RenderDeviceGLES2::destroyBuffer(uint32_t buffer)
{
    if (buffer == 0) return;

    RDIBuffer& buf = mBuffers.getRef(buffer);
    if (buf.glObj != 0) glDeleteBuffers(1, &buf.glObj);

    mBufferMemory -= buf.size;
    mBuffers.remove(buffer);
}

//----------------------------------------------------------
bool RenderDeviceGLES2::updateBufferData(uint32_t buffer, uint32_t offset, uint32_t size,
    const void* data)
{
    const RDIBuffer& buf = mBuffers.getRef(buffer);
    if (offset + size > buf.size) return false;

    glBindBuffer(buf.type, buf.glObj);
    if (offset == 0 && size == buf.size) {
        // Replacing the whole buffer can help the driver to avoid pipeline stalls
        glBufferData(buf.type, size, data, GL_DYNAMIC_DRAW);
    }
    else {
        glBufferSubData(buf.type, offset, size, data);
    }

    return true;
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::getBufferMemory() const
{
    return mBufferMemory;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setViewport(int x, int y, int width, int height)
{
    mVpX      = x;
    mVpY      = y;
    mVpWidth  = width;
    mVpHeight = height;

    mPendingMask |= PMViewport;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setScissorRect(int x, int y, int width, int height)
{
    mScX      = x;
    mScY      = y;
    mScWidth  = width;
    mScHeight = height;

    mPendingMask |= PMScissor;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setIndexBuffer(uint32_t bufObj, RDIIndexFormat format)
{
    mIndexFormat = toIndexFormat[format];
    mNewIndexBuffer = bufObj;
    mPendingMask |= PMIndexBuffer;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setVertexBuffer(uint32_t slot, uint32_t vbObj, uint32_t offset,
    uint32_t stride)
{
    if (slot >= 16) {
        Log::warning("Attempting to set Vertex buffer at slot >= 16");
        return;
    }

    mVertBufSlots[slot] = {vbObj, offset, stride};
    mPendingMask |= PMVertexLayout;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setVertexLayout(uint32_t vlObj)
{
    mNewVertexLayout = vlObj;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setTexture(uint32_t slot, uint32_t texObj, uint16_t samplerState)
{
    mTexSlots[slot] = {texObj, samplerState};
    mPendingMask |= PMTextures;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setColorWriteMask(bool enabled)
{
    mNewRasterState.renderTargetWriteMask = enabled;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::getColorWriteMask() const
{
    return mNewRasterState.renderTargetWriteMask;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setFillMode(RDIFillMode fillMode)
{
    mNewRasterState.fillMode = fillMode;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
RDIFillMode RenderDeviceGLES2::getFillMode() const
{
    return static_cast<RDIFillMode>(mNewRasterState.fillMode);
}

//----------------------------------------------------------
void RenderDeviceGLES2::setCullMode(RDICullMode cullMode)
{
    mNewRasterState.cullMode = cullMode;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
RDICullMode RenderDeviceGLES2::getCullMode() const
{
    return static_cast<RDICullMode>(mNewRasterState.cullMode);
}

//----------------------------------------------------------
void RenderDeviceGLES2::setScissorTest(bool enabled)
{
    mNewRasterState.scissorEnable = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::getScissorTest() const
{
    return mNewRasterState.scissorEnable;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setMultisampling(bool enabled)
{
    mNewRasterState.multisampleEnable = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::getMultisampling() const
{
    return mNewRasterState.multisampleEnable;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setAlphaToCoverage(bool enabled)
{
    mNewBlendState.alphaToCoverageEnable = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::getAlphaToCoverage() const
{
    return mNewBlendState.alphaToCoverageEnable;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setBlendMode(bool enabled, RDIBlendFunc src, RDIBlendFunc dst)
{
    mNewBlendState.blendEnable = enabled;
    mNewBlendState.srcBlendFunc = src;
    mNewBlendState.dstBlendFunc = dst;

    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::getBlendMode(RDIBlendFunc& src, RDIBlendFunc& dst) const
{
    src = static_cast<RDIBlendFunc>(mNewBlendState.srcBlendFunc);
    dst = static_cast<RDIBlendFunc>(mNewBlendState.dstBlendFunc);
    return mNewBlendState.blendEnable;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setDepthMask(bool enabled)
{
    mNewDepthStencilState.depthWriteMask = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::getDepthMask() const
{
    return mNewDepthStencilState.depthWriteMask;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setDepthTest(bool enabled)
{
    mNewDepthStencilState.depthEnable = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::getDepthTest() const
{
    return mNewDepthStencilState.depthEnable;
}

//----------------------------------------------------------
void RenderDeviceGLES2::setDepthFunc(RDIDepthFunc depthFunc)
{
    mNewDepthStencilState.depthFunc = depthFunc;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
RDIDepthFunc RenderDeviceGLES2::getDepthFunc() const
{
    return static_cast<RDIDepthFunc>(mNewDepthStencilState.depthFunc);
}

//----------------------------------------------------------
bool RenderDeviceGLES2::isTextureCompressionSupported() const
{
    std::lock_guard<std::mutex> lock(cpMutex);
    return glExt::EXT_texture_compression_s3tc;
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::createShaderProgram(const char* vertexShaderSrc,
    const char* fragmentShaderSrc)
{
    int infoLogLength {0};
    int charsWritten  {0};
    char* infoLog     {nullptr};
    int status;

    shaderLog = "";

    // Vertex shader
    uint32_t vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSrc, nullptr);
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
        return 0;
    }

    // Fragment shader
    uint32_t fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSrc, nullptr);
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
        return 0;
    }

    // Shader program
    uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::linkShaderProgram(uint32_t programObj)
{
    int infoLogLength {0};
    int charsWritten {0};
    char* infoLog {nullptr};
    int status;

    shaderLog = "";

    glLinkProgram(programObj);
    glGetProgramiv(programObj, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 1) {
        infoLog = new char[infoLogLength];
        glGetProgramInfoLog(programObj, infoLogLength, &charsWritten, infoLog);
        shaderLog = shaderLog + "[Linking]\n" + infoLog;
        delete[] infoLog;
        infoLog = nullptr;
    }

    glGetProgramiv(programObj, GL_LINK_STATUS, &status);
    if (!status) return false;

    return true;
}

//----------------------------------------------------------
bool RenderDeviceGLES2::applyVertexLayout()
{
    uint32_t newVertexAttribMask {0u};

    if (mNewVertexLayout != 0) {
        if (mCurShaderID == 0) return false;

        RDIVertexLayout& v1         = mVertexLayouts[mNewVertexLayout - 1];
        RDIShader& shader           = mShaders.getRef(mCurShaderID);
        RDIInputLayout& inputLayout = shader.inputLayouts[mNewVertexLayout - 1];

        if (!inputLayout.valid) return false;

        // Set vertex attrib pointers
        for (uint32_t i = 0; i < v1.numAttribs; ++i) {
            int8_t attribIndex = inputLayout.attribIndices[i];
            if (attribIndex >= 0) {
                VertexLayoutAttrib& attrib = v1.attribs[i];
                const auto& vbSlot = mVertBufSlots[attrib.vbSlot];

                if (mBuffers.getRef(vbSlot.vbObj).glObj == 0 ||
                    mBuffers.getRef(vbSlot.vbObj).type != GL_ARRAY_BUFFER) {
                    Log::error("Attempting to reference an invalid vertex buffer");
                    return false;
                }

                glBindBuffer(GL_ARRAY_BUFFER, mBuffers.getRef(vbSlot.vbObj).glObj);
                glVertexAttribPointer(attribIndex, attrib.size, GL_FLOAT, GL_FALSE,
                    vbSlot.stride, (char*)0 + vbSlot.offset + attrib.offset);

                newVertexAttribMask |= 1 << attribIndex;
            }
        }

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

        return true;
    }

    // TODO
    return true;
}

//----------------------------------------------------------
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

    auto filter = (state & SamplerState::FilterMask) >> SamplerState::FilterStart;
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, tex.hasMips ?
        magFilters[filter] : minFiltersMips[filter]);

    filter = (state & SamplerState::FilterMask) >> SamplerState::FilterStart;
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilters[filter]);

    // TODO: Check for anisotropy availability
    // filter = (state & SamplerState::AnisoMask) >> SamplerState::AnisoStart;
    // glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso[filter]);

    filter = (state & SamplerState::AddrUMask) >> SamplerState::AddrUStart;
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapModes[filter]);

    filter = (state & SamplerState::AddrVMask) >> SamplerState::AddrVStart;
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapModes[filter]);

    // TODO: Check for Texture3D availability
    // filter = (state & SamplerState::AddrWMask) >> SamplerState::AddrWStart;
    // glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapModes[filter]);

    // TODO: Check for shadow samplers availability
    // if (!(state & SamplerState::CompLEqual)) {
    //     glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    // }
    // else {
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    // }
}

//----------------------------------------------------------
void RenderDeviceGLES2::applyRenderStates()
{
    // Rasterizer state
    if (mNewRasterState.hash != mCurRasterState.hash) {
        // TODO: Not supported on GLES2
        // if (mNewRasterState.fillMode == RS_FILL_SOLID) {
        //     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // }
        // else {
        //     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // }

        if (mNewRasterState.cullMode == RS_CULL_BACK) {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else if (mNewRasterState.cullMode == RS_CULL_FRONT) {
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
            glBlendFunc(oglBlendFuncs[mNewBlendState.srcBlendFunc],
                oglBlendFuncs[mNewBlendState.dstBlendFunc]);
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

//------------------------------------------------------------------------------
#endif
//==============================================================================
