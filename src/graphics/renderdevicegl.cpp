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
#include "renderdevicegl.hpp"

#if !defined(NX_OPENGL_ES)
//------------------------------------------------------------------------------
#include "../system/log.hpp"
#include "opengl.hpp"

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
    "uniform vec4 color;\n"
    "void main() {\n"
    "   gl_FragColor = color;\n"
    "}\n";

static uint32_t toIndexFormat[] = {GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};
static uint32_t toPrimType[]    = {GL_TRIANGLES, GL_TRIANGLE_STRIP};
static int      toTexType[]     = {GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP};

static std::mutex vlMutex; // Vertex layouts mutex
static std::mutex txMutex; // Texture operations mutex
static std::mutex cpMutex; // Capabilities mutex
thread_local std::string shaderLog;

//----------------------------------------------------------
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
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &mMaxCubeTextureSize);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &mMaxTextureUnits);

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

    // Set some default values
    mIndexFormat = GL_UNSIGNED_SHORT;
    mActiveVertexAttribsMask = 0;

    // TODO: Find supported depth format (some old ATI cards only support 16 bit depth for FBOs)
    mDepthFormat = GL_DEPTH_COMPONENT24;

    initStates();
    resetStates();

    return true;
}

//----------------------------------------------------------
void RenderDeviceGL::initStates()
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

//----------------------------------------------------------
void RenderDeviceGL::resetStates()
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
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
}

//----------------------------------------------------------
bool RenderDeviceGL::commitStates(uint32_t filter)
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
            for (uint32_t i = 0; i < 16; ++i) {
                glActiveTexture(GL_TEXTURE0 + i);
                auto& texSlot = mTexSlots[i];

                if (texSlot.texObj == 0) {
                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    glBindTexture(GL_TEXTURE_3D, 0);
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
void RenderDeviceGL::clear(const float* color)
{
    commitStates(PMViewport | PMScissor | PMRenderStates);
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------
void RenderDeviceGL::draw(RDIPrimType primType, uint32_t firstVert, uint32_t vertCount)
{
    if (commitStates()) {
        glDrawArrays(toPrimType[primType], firstVert, vertCount);
    }
}

//----------------------------------------------------------
void RenderDeviceGL::drawIndexed(RDIPrimType primType, uint32_t firstIndex, uint32_t indexCount)
{
    if (commitStates()) {
        firstIndex *= (mIndexFormat == GL_UNSIGNED_SHORT) ? sizeof(short) : sizeof(int);

        glDrawElements(toPrimType[primType], indexCount, mIndexFormat, (char*)0 + firstIndex);
    }
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::registerVertexLayout(uint32_t numAttribs,
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
uint32_t RenderDeviceGL::createTexture(TextureType::Type type, int width, int height,
    unsigned int depth, TextureFormat::Type format, bool hasMips, bool genMips, bool sRGB)
{
    if (!mTexNPOTSupported && ((width & (width-1)) != 0 || ((height & (height-1)) != 0))) {
        Log::error("Non-Power-Of-Two textures are not supported by GPU");
        return 0;
    }

    if (type == TextureType::TexCube && (width > mMaxCubeTextureSize || height > mMaxTextureSize)) {
        Log::error("Cube map is bigger than limit size");
        return 0;
    }
    else if (type != TextureType::TexCube && (width > mMaxTextureSize || height > mMaxTextureSize ||
        depth > static_cast<unsigned int>(mMaxTextureSize)))
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
        tex.glFmt = tex.sRGB ? GL_SRGB8_ALPHA8_EXT : GL_RGBA8;
        break;
    case TextureFormat::DXT1:
        tex.glFmt = tex.sRGB ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case TextureFormat::DXT3:
        tex.glFmt = tex.sRGB ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case TextureFormat::DXT5:
        tex.glFmt = tex.sRGB ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    case TextureFormat::RGBA16F:
        tex.glFmt = GL_RGBA16F_ARB;
        break;
    case TextureFormat::RGBA32F:
        tex.glFmt = GL_RGBA32F_ARB;
        break;
    case TextureFormat::DEPTH:
        tex.glFmt = mDepthFormat;
    default:
        Log::warning("Could not create texture: invalid format");
        return 0;
    }

    glGenTextures(1, &tex.glObj);
    if (tex.glObj == 0) return 0;

    glActiveTexture(GL_TEXTURE15);
    glBindTexture(tex.type, tex.glObj);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    tex.samplerState = 0;
    applySamplerState(tex);

    glBindTexture(tex.type, 0);
    {
        std::lock_guard<std::mutex> lock(txMutex);
        if (mTexSlots[15].texObj) {
            auto& tex15 = mTextures.getRef(mTexSlots[15].texObj);
            glBindTexture(tex15.type, tex15.glObj);
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
void RenderDeviceGL::uploadTextureData(uint32_t texObj, int slice, int mipLevel, const void* pixels)
{
    if (texObj == 0) return;

    const auto& tex = mTextures.getRef(texObj);
    auto format     = tex.format;

    glActiveTexture(GL_TEXTURE15);
    glBindTexture(tex.type, tex.glObj);

    int inputFormat = GL_RGBA, inputType = GL_UNSIGNED_BYTE;
    bool compressed = (format == TextureFormat::DXT1) || (format == TextureFormat::DXT3) ||
                      (format == TextureFormat::DXT5);

    switch (format) {
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            inputType = GL_FLOAT;
            break;
        case TextureFormat::DEPTH:
            inputFormat = GL_DEPTH_COMPONENT;
            inputType = GL_FLOAT;
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
    else if (tex.type == GL_TEXTURE_3D) {
        int depth = std::max(tex.depth >> mipLevel, 1);

        if (compressed) {
            glCompressedTexImage3D(GL_TEXTURE_3D, mipLevel, tex.glFmt, width, height, depth, 0,
                calcTextureSize(format, width, height, depth), pixels);
        }
        else {
            glTexImage3D(GL_TEXTURE_3D, mipLevel, tex.glFmt, width, height, depth, 0, inputFormat,
                inputType, pixels);
        }
    }

    if (tex.genMips && (tex.type != GL_TEXTURE_CUBE_MAP || slice == 5)) {
        // Note: cube map mips are only generated when the last side is uploaded
        glEnable(tex.type);
        glGenerateMipmapEXT(tex.type);
        glDisable(tex.type);
    }

    glBindTexture(tex.type, 0);
    {
        std::lock_guard<std::mutex> lock(txMutex);
        if (mTexSlots[15].texObj) {
            auto& tex15 = mTextures.getRef(mTexSlots[15].texObj);
            glBindTexture(tex15.type, tex15.glObj);
        }
    }
}

//----------------------------------------------------------
void RenderDeviceGL::uploadTextureSubData(uint32_t texObj, int slice, int mipLevel, unsigned int x,
    unsigned int y, unsigned int z, unsigned int width, unsigned int height, unsigned int depth,
    const void* pixels)
{
    if (texObj == 0) return;

    const auto& tex = mTextures.getRef(texObj);
    auto format     = tex.format;

    if (x + width > static_cast<unsigned int>(tex.width) ||
        y + height > static_cast<unsigned int>(tex.height)) {
        Log::info("Attemting to update portion out of texture boundaries");
        return;
    }

    glActiveTexture(GL_TEXTURE15);
    glBindTexture(tex.type, tex.glObj);

    int inputFormat = GL_RGBA, inputType = GL_UNSIGNED_BYTE;
    bool compressed = (format == TextureFormat::DXT1) || (format == TextureFormat::DXT3) ||
                      (format == TextureFormat::DXT5);

    switch (format) {
        case TextureFormat::RGBA16F:
        case TextureFormat::RGBA32F:
            inputType = GL_FLOAT;
            break;
        case TextureFormat::DEPTH:
            inputFormat = GL_DEPTH_COMPONENT;
            inputType = GL_FLOAT;
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
    else if (tex.type == GL_TEXTURE_3D) {
        if (compressed) {
            glCompressedTexSubImage3D(GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth,
                tex.glFmt, calcTextureSize(format, width, height, depth), pixels);
        }
        else {
            glTexSubImage3D(GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth, inputFormat,
                inputType, pixels);
        }
    }

    if (tex.genMips && (tex.type != GL_TEXTURE_CUBE_MAP || slice == 5)) {
        // Note: cube map mips are only generated when the last side is uploaded
        glEnable(tex.type);
        glGenerateMipmapEXT(tex.type);
        glDisable(tex.type);
    }

    glBindTexture(tex.type, 0);
    {
        std::lock_guard<std::mutex> lock(txMutex);
        if (mTexSlots[15].texObj) {
            auto& tex15 = mTextures.getRef(mTexSlots[15].texObj);
            glBindTexture(tex15.type, tex15.glObj);
        }
    }
}

//----------------------------------------------------------
void RenderDeviceGL::destroyTexture(uint32_t texObj)
{
    if (texObj == 0) return;
    const auto& tex = mTextures.getRef(texObj);

    glDeleteTextures(1, &tex.glObj);
    mTextureMemory -= tex.memSize;
    mTextures.remove(texObj);
}

//----------------------------------------------------------
bool RenderDeviceGL::getTextureData(uint32_t texObj, int slice, int mipLevel, void* buffer)
{
    const auto& tex = mTextures.getRef(texObj);

    int target = (tex.type == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    if (target == GL_TEXTURE_CUBE_MAP) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;

    int fmt, type, compressed = 0;
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(tex.type, tex.glObj);

    switch(tex.format) {
    case TextureFormat::RGBA8:
        fmt = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
    case TextureFormat::DXT1:
    case TextureFormat::DXT3:
    case TextureFormat::DXT5:
        compressed = 1;
        break;
    default:
        return false;
    };

    if (compressed) {
        glGetCompressedTexImage(target, mipLevel, buffer);
    }
    else {
        glGetTexImage(target, mipLevel, fmt, type, buffer);
    }

    glBindTexture(tex.type, 0);
    {
        std::lock_guard<std::mutex> lock(txMutex);
        if (mTexSlots[15].texObj) {
            auto& tex15 = mTextures.getRef(mTexSlots[15].texObj);
            glBindTexture(tex15.type, tex15.glObj);
        }
    }

    return true;
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::getTextureMemory() const
{
    return mTextureMemory;
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::createShader(const char* vertexShaderSrc, const char* fragmentShaderSrc)
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
void RenderDeviceGL::destroyShader(uint32_t shaderID)
{
    if (shaderID == 0) return;
    RDIShader &shader = mShaders.getRef(shaderID);
    glDeleteProgram(shader.oglProgramObj);
    mShaders.remove(shaderID);
}

//----------------------------------------------------------
void RenderDeviceGL::bindShader(uint32_t shaderID)
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
const std::string& RenderDeviceGL::getShaderLog()
{
    return shaderLog;
}

//----------------------------------------------------------
int RenderDeviceGL::getShaderConstLoc(uint32_t shaderID, const char* name)
{
    RDIShader& shader = mShaders.getRef(shaderID);
    return glGetUniformLocation(shader.oglProgramObj, name);
}

//----------------------------------------------------------
int RenderDeviceGL::getShaderSamplerLoc(uint32_t shaderID, const char* name)
{
    RDIShader& shader = mShaders.getRef(shaderID);
    return glGetUniformLocation(shader.oglProgramObj, name);
}

//----------------------------------------------------------
void RenderDeviceGL::setShaderConst(int loc, RDIShaderConstType type, float* values, uint32_t count)
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
void RenderDeviceGL::setShaderSampler(int loc, uint32_t texUnit)
{
    glUniform1i(loc, (int)texUnit);
}

//----------------------------------------------------------
const char* RenderDeviceGL::getDefaultVSCode()
{
    return defaultShaderVS;
}

//----------------------------------------------------------
const char* RenderDeviceGL::getDefaultFSCode()
{
    return defaultShaderFS;
}

//----------------------------------------------------------
void RenderDeviceGL::beginRendering()
{
    // Get the currently bound frame buffer object. 
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &mDefaultFBO);

    resetStates();
}

//----------------------------------------------------------
void RenderDeviceGL::finishRendering()
{
    // Nothing to do
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::createVertexBuffer(uint32_t size, const void* data)
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
uint32_t RenderDeviceGL::createIndexBuffer(uint32_t size, const void* data)
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
void RenderDeviceGL::destroyBuffer(uint32_t buffer)
{
    if (buffer == 0) return;

    RDIBuffer& buf = mBuffers.getRef(buffer);
    glDeleteBuffers(1, &buf.glObj);

    mBufferMemory -= buf.size;
    mBuffers.remove(buffer);
}

//----------------------------------------------------------
bool RenderDeviceGL::updateBufferData(uint32_t buffer, uint32_t offset, uint32_t size,
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
uint32_t RenderDeviceGL::getBufferMemory() const
{
    return mBufferMemory;
}

//----------------------------------------------------------
void RenderDeviceGL::setViewport(int x, int y, int width, int height)
{
    mVpX      = x;
    mVpY      = y;
    mVpWidth  = width;
    mVpHeight = height;

    mPendingMask |= PMViewport;
}

//----------------------------------------------------------
void RenderDeviceGL::setScissorRect(int x, int y, int width, int height)
{
    mScX      = x;
    mScY      = y;
    mScWidth  = width;
    mScHeight = height;

    mPendingMask |= PMScissor;
}

//----------------------------------------------------------
void RenderDeviceGL::setIndexBuffer(uint32_t bufObj, RDIIndexFormat format)
{
    mIndexFormat = toIndexFormat[format];
    mNewIndexBuffer = bufObj;
    mPendingMask |= PMIndexBuffer;
}

//----------------------------------------------------------
void RenderDeviceGL::setVertexBuffer(uint32_t slot, uint32_t vbObj, uint32_t offset,
    uint32_t stride)
{
    mVertBufSlots[slot] = {vbObj, offset, stride};
    mPendingMask |= PMVertexLayout;
}

//----------------------------------------------------------
void RenderDeviceGL::setVertexLayout(uint32_t vlObj)
{
    mNewVertexLayout = vlObj;
}

//----------------------------------------------------------
void RenderDeviceGL::setTexture(uint32_t slot, uint32_t texObj, uint16_t samplerState)
{
    mTexSlots[slot] = {texObj, samplerState};
    mPendingMask |= PMTextures;
}

//----------------------------------------------------------
void RenderDeviceGL::setColorWriteMask(bool enabled)
{
    mNewRasterState.renderTargetWriteMask = enabled;
}

//----------------------------------------------------------
bool RenderDeviceGL::getColorWriteMask() const
{
    return mNewRasterState.renderTargetWriteMask;
}

//----------------------------------------------------------
void RenderDeviceGL::setFillMode(RDIFillMode fillMode)
{
    mNewRasterState.fillMode = fillMode;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
RDIFillMode RenderDeviceGL::getFillMode() const
{
    return static_cast<RDIFillMode>(mNewRasterState.fillMode);
}

//----------------------------------------------------------
void RenderDeviceGL::setCullMode(RDICullMode cullMode)
{
    mNewRasterState.cullMode = cullMode;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
RDICullMode RenderDeviceGL::getCullMode() const
{
    return static_cast<RDICullMode>(mNewRasterState.cullMode);
}

//----------------------------------------------------------
void RenderDeviceGL::setScissorTest(bool enabled)
{
    mNewRasterState.scissorEnable = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGL::getScissorTest() const
{
    return mNewRasterState.scissorEnable;
}

//----------------------------------------------------------
void RenderDeviceGL::setMultisampling(bool enabled)
{
    mNewRasterState.multisampleEnable = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGL::getMultisampling() const
{
    return mNewRasterState.multisampleEnable;
}

//----------------------------------------------------------
void RenderDeviceGL::setAlphaToCoverage(bool enabled)
{
    mNewBlendState.alphaToCoverageEnable = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGL::getAlphaToCoverage() const
{
    return mNewBlendState.alphaToCoverageEnable;
}

//----------------------------------------------------------
void RenderDeviceGL::setBlendMode(bool enabled, RDIBlendFunc src, RDIBlendFunc dst)
{
    mNewBlendState.blendEnable = enabled;
    mNewBlendState.srcBlendFunc = src;
    mNewBlendState.dstBlendFunc = dst;

    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGL::getBlendMode(RDIBlendFunc& src, RDIBlendFunc& dst) const
{
    src = static_cast<RDIBlendFunc>(mNewBlendState.srcBlendFunc);
    dst = static_cast<RDIBlendFunc>(mNewBlendState.dstBlendFunc);
    return mNewBlendState.blendEnable;
}

//----------------------------------------------------------
void RenderDeviceGL::setDepthMask(bool enabled)
{
    mNewDepthStencilState.depthWriteMask = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGL::getDepthMask() const
{
    return mNewDepthStencilState.depthWriteMask;
}

//----------------------------------------------------------
void RenderDeviceGL::setDepthTest(bool enabled)
{
    mNewDepthStencilState.depthEnable = enabled;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
bool RenderDeviceGL::getDepthTest() const
{
    return mNewDepthStencilState.depthEnable;
}

//----------------------------------------------------------
void RenderDeviceGL::setDepthFunc(RDIDepthFunc depthFunc)
{
    mNewDepthStencilState.depthFunc = depthFunc;
    mPendingMask |= PMRenderStates;
}

//----------------------------------------------------------
RDIDepthFunc RenderDeviceGL::getDepthFunc() const
{
    return static_cast<RDIDepthFunc>(mNewDepthStencilState.depthFunc);
}

//----------------------------------------------------------
void RenderDeviceGL::getCapabilities(unsigned int& maxTexUnits, unsigned int& maxTexSize,
        unsigned int& maxCubTexSize, bool& dxt, bool& pvrtci, bool& etc1, bool& texFloat,
        bool& texDepth, bool& texSS, bool& tex3D, bool& texNPOT, bool& texSRGB, bool& rtms,
        bool& occQuery, bool& timerQuery) const
{
    std::lock_guard<std::mutex> lock(cpMutex);
    maxTexUnits   = mMaxTextureUnits;
    maxTexSize    = mMaxTextureSize;
    maxCubTexSize = mMaxCubeTextureSize;
    dxt           = mDXTSupported;
    pvrtci        = mPVRTCISupported;
    etc1          = mTexETC1Supported;
    texFloat      = mTexFloatSupported;
    texDepth      = mTexDepthSupported;
    texSS         = mTexShadowSamplers;
    tex3D         = mTex3DSupported;
    texNPOT       = mTexNPOTSupported;
    texSRGB       = mTexSRGBSupported;
    rtms          = mRTMultiSampling;
    occQuery      = mOccQuerySupported;
    timerQuery    = mTimerQuerySupported;
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::createShaderProgram(const char* vertexShaderSrc,
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
bool RenderDeviceGL::linkShaderProgram(uint32_t programObj)
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
bool RenderDeviceGL::applyVertexLayout()
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
void RenderDeviceGL::applySamplerState(RDITexture& tex)
{
    thread_local const uint32_t magFilters[] = {GL_LINEAR, GL_LINEAR, GL_NEAREST};
    thread_local const uint32_t minFiltersMips[] = {
        GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST
    };
    thread_local const uint32_t maxAniso[] = {1u, 2u, 4u, 8u, 16u, 1u, 1u, 1u};
    thread_local const uint32_t wrapModes[] = {GL_CLAMP_TO_EDGE, GL_REPEAT, GL_CLAMP_TO_BORDER};

    uint32_t state = tex.samplerState;
    uint32_t target = tex.type;

    auto filter = (state & SamplerState::FilterMask) >> SamplerState::FilterStart;
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, tex.hasMips ?
        magFilters[filter] : minFiltersMips[filter]);

    filter = (state & SamplerState::FilterMask) >> SamplerState::FilterStart;
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilters[filter]);

    filter = (state & SamplerState::AnisoMask) >> SamplerState::AnisoStart;
    glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso[filter]);

    filter = (state & SamplerState::AddrUMask) >> SamplerState::AddrUStart;
    glTexParameteri(target, GL_TEXTURE_WRAP_S, wrapModes[filter]);

    filter = (state & SamplerState::AddrVMask) >> SamplerState::AddrVStart;
    glTexParameteri(target, GL_TEXTURE_WRAP_T, wrapModes[filter]);

    filter = (state & SamplerState::AddrWMask) >> SamplerState::AddrWStart;
    glTexParameteri(target, GL_TEXTURE_WRAP_R, wrapModes[filter]);

    if (!(state & SamplerState::CompLEqual)) {
        glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    }
    else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }
}

//----------------------------------------------------------
void RenderDeviceGL::applyRenderStates()
{
    // Rasterizer state
    if (mNewRasterState.hash != mCurRasterState.hash) {
        if (mNewRasterState.fillMode == RS_FILL_SOLID) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

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
