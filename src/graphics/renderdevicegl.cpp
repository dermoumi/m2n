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

#include <algorithm>

// Locals
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

static GLenum toVertexFormat[] = {GL_FLOAT, GL_UNSIGNED_BYTE};
static GLenum toIndexFormat[]  = {GL_UNSIGNED_SHORT, GL_UNSIGNED_INT};
static GLenum toTexType[]      = {GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP};
static GLenum toTexBinding[]   = {
    GL_TEXTURE_BINDING_2D, GL_TEXTURE_BINDING_3D, GL_TEXTURE_BINDING_CUBE_MAP
};
static GLenum toPrimType[]     = {
    GL_POINTS, GL_LINES, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLES, GL_TRIANGLE_STRIP,
    GL_TRIANGLE_FAN
};

static std::string shaderLog;

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
    if (mMaxTextureUnits > 16) mMaxTextureUnits = 16;

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &mMaxColBuffers);
    if (mMaxColBuffers > 4) mMaxColBuffers = 4;

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
    uint32_t testBuf = createRenderBuffer(32, 32, RGBA8, true, 1, 0);
    if (testBuf == 0) {
        mDepthFormat = GL_DEPTH_COMPONENT16;
        Log::warning("Render target depth precision limited to 16 bits");
    }
    else {
        destroyRenderBuffer(testBuf);
    }

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
    mCurVertexLayout = 1;                     mNewVertexLayout = 0;
    mCurIndexBuffer = 1;                      mNewIndexBuffer = 0;
    mCurRasterState.hash = 0xFFFFFFFFu;       mNewRasterState.hash = 0u;
    mCurBlendState.hash = 0xFFFFFFFFu;        mNewBlendState.hash = 0u;
    mCurDepthStencilState.hash = 0xFFFFFFFFu; mCurDepthStencilState.hash = 0u;

    for (uint32_t i = 0; i < 16; ++i) setTexture(i, 0, 0);

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
        if (mask & IndexBuffer) {
            if (mNewIndexBuffer != mCurIndexBuffer) {
                if (mNewIndexBuffer == 0) {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                }
                else {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffers.getRef(mNewIndexBuffer).glObj);
                }

                mCurIndexBuffer = mNewIndexBuffer;
            }

            mPendingMask &= ~IndexBuffer;
        }

        // Bind textures and set sampler state
        if (mask & Textures) {
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

            mPendingMask &= ~Textures;
        }

        // Bind vertex buffers
        if (mask & VertexLayouts) {
            if (
                mNewVertexLayout != mCurVertexLayout || mPrevShaderID != mCurShaderID ||
                mVertexBufUpdated
            ) {
                if (!applyVertexLayout()) return false;

                mCurVertexLayout  = mNewVertexLayout;
                mPrevShaderID     = mCurShaderID;
                mVertexBufUpdated = false;
            }

            mPendingMask &= ~VertexLayouts;
        }
    }

    return true;
}

void RenderDeviceGL::clear(uint32_t flags, const float* color, float depth)
{
    uint32_t prevBuffers[4] = {0u};

    if (mCurRenderBuffer != 0) {
        auto& rb = mRenderBuffers.getRef(mCurRenderBuffer);

        if ((flags & ClrDepth) && rb.depthTex == 0) flags &= ~ClrDepth;

        for (uint32_t i = 0; i < 4; ++i) {
            glGetIntegerv(GL_DRAW_BUFFER0 + i, reinterpret_cast<int*>(&prevBuffers[i]));
        }

        uint32_t buffers[4], cnt = 0;
        if (flags & ClrColorRT0 && rb.colTexs[0] != 0) buffers[cnt++] = GL_COLOR_ATTACHMENT0_EXT;
        if (flags & ClrColorRT1 && rb.colTexs[1] != 0) buffers[cnt++] = GL_COLOR_ATTACHMENT1_EXT;
        if (flags & ClrColorRT2 && rb.colTexs[2] != 0) buffers[cnt++] = GL_COLOR_ATTACHMENT2_EXT;
        if (flags & ClrColorRT3 && rb.colTexs[3] != 0) buffers[cnt++] = GL_COLOR_ATTACHMENT3_EXT;

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

    if (mCurRenderBuffer != 0) {
        glDrawBuffers(4, prevBuffers);
    }
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

uint32_t RenderDeviceGL::registerVertexLayout(uint8_t numAttribs,
    const VertexLayoutAttrib* attribs)
{
    if (mNumVertexLayouts == MaxNumVertexLayouts) return 0;

    mVertexLayouts[mNumVertexLayouts].numAttribs = numAttribs;
    for (uint8_t i = 0; i < numAttribs; ++i) {
        mVertexLayouts[mNumVertexLayouts].attribs[i] = attribs[i];
    }

    return ++mNumVertexLayouts;
}

uint32_t RenderDeviceGL::createTexture(TextureType type, int width, int height,
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

    if (format == PVRTCI_2BPP || format == PVRTCI_A2BPP || format == PVRTCI_4BPP ||
        format == PVRTCI_A4BPP || format == ETC1)
    {
        Log::error("PVRTCI and ETC1 texture formats are not supported by the GPU");
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
        tex.glFmt = tex.sRGB ? GL_SRGB8_ALPHA8_EXT : GL_RGBA8;
        break;
    case DXT1:
        tex.glFmt = tex.sRGB ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case DXT3:
        tex.glFmt = tex.sRGB ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case DXT5:
        tex.glFmt = tex.sRGB ?
            GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    case RGBA16F:
        tex.glFmt = GL_RGBA16F_ARB;
        break;
    case RGBA32F:
        tex.glFmt = GL_RGBA32F_ARB;
        break;
    case DEPTH:
        tex.glFmt = mDepthFormat;
        break;
    default:
        Log::error("Could not create texture: invalid format");
        return 0;
    }

    glGenTextures(1, &tex.glObj);
    if (tex.glObj == 0) {
        Log::error("Could not generate GPU texture");
        return 0;
    }

    glActiveTexture(GL_TEXTURE15);

    int lastTexture;
    glGetIntegerv(toTexBinding[tex.type], &lastTexture);

    glBindTexture(tex.type, tex.glObj);

    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    tex.samplerState = 0;
    applySamplerState(tex);

    glBindTexture(tex.type, lastTexture);

    // Calculate memory requirements
    tex.memSize = calcTextureSize(format, width, height, depth);
    if (hasMips || genMips) tex.memSize += static_cast<int>(tex.memSize / 3.f + 0.5f);
    if (type == TextureType::TexCube) tex.memSize *= 6;
    mTextureMemory += tex.memSize;

    return mTextures.add(tex, true);
}

void RenderDeviceGL::uploadTextureData(uint32_t texObj, int slice, int mipLevel, const void* pixels)
{
    if (texObj == 0) return;

    const auto& tex = mTextures.getRef(texObj);
    auto format     = tex.format;

    glActiveTexture(GL_TEXTURE15);

    int lastTexture;
    glGetIntegerv(toTexBinding[tex.type], &lastTexture);

    glBindTexture(tex.type, tex.glObj);

    int inputFormat = GL_RGBA;
    int inputType = GL_UNSIGNED_BYTE;
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

    glBindTexture(tex.type, lastTexture);
}

void RenderDeviceGL::uploadTextureSubData(uint32_t texObj, int slice, int mipLevel, unsigned int x,
    unsigned int y, unsigned int z, unsigned int width, unsigned int height, unsigned int depth,
    const void* pixels)
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

    glActiveTexture(GL_TEXTURE15);

    int lastTexture;
    glGetIntegerv(toTexBinding[tex.type], &lastTexture);

    glBindTexture(tex.type, tex.glObj);

    int inputFormat = GL_RGBA;
    int inputType = GL_UNSIGNED_BYTE;
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

    glBindTexture(tex.type, lastTexture);
}

void RenderDeviceGL::destroyTexture(uint32_t texObj)
{
    if (texObj == 0) return;
    const auto& tex = mTextures.getRef(texObj);

    glDeleteTextures(1, &tex.glObj);
    mTextureMemory -= tex.memSize;

    mTextures.remove(texObj);
}

bool RenderDeviceGL::getTextureData(uint32_t texObj, int slice, int mipLevel, void* buffer)
{
    const auto& tex = mTextures.getRef(texObj);

    int target = (tex.type == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    if (target == GL_TEXTURE_CUBE_MAP) target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + slice;

    int fmt, type, compressed = 0;
    glActiveTexture(GL_TEXTURE15);

    int lastTexture;
    glGetIntegerv(toTexBinding[tex.type], &lastTexture);

    glBindTexture(tex.type, tex.glObj);

    switch(tex.format) {
    case RGBA8:
        fmt = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
    case DXT1:
    case DXT3:
    case DXT5:
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

    glBindTexture(tex.type, lastTexture);

    return true;
}

uint32_t RenderDeviceGL::getTextureMemory() const
{
    return mTextureMemory;
}

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
        auto& vl = mVertexLayouts[i];

        // Reset attribute indices to -1 (no attribute)
        for (uint32_t j = 0; j < 16u; ++j) {
            shader.inputLayouts[i].attribIndices[j] = -1;
        }

        // Check if shader has all declared attributes, and set locations
        for (int j = 0; j < attribCount; ++j)
        {
            char name[32];
            uint32_t size, type;
            glGetActiveAttrib(programObj, j, 32, nullptr, (int*)&size, &type, name);

            bool attribFound = false;
            for (uint32_t k = 0; k < vl.numAttribs; ++k) {
                if (vl.attribs[k].semanticName == name) {
                    auto loc = glGetAttribLocation(programObj, name);
                    shader.inputLayouts[i].attribIndices[k] = loc;
                    attribFound = true;
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

void RenderDeviceGL::destroyShader(uint32_t shaderID)
{
    if (shaderID == 0) return;
    RDIShader &shader = mShaders.getRef(shaderID);
    glDeleteProgram(shader.oglProgramObj);
    mShaders.remove(shaderID);
}

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
    mPendingMask |= VertexLayouts;
}

const std::string& RenderDeviceGL::getShaderLog()
{
    return shaderLog;
}

int RenderDeviceGL::getShaderConstLoc(uint32_t shaderID, const char* name)
{
    RDIShader& shader = mShaders.getRef(shaderID);
    return glGetUniformLocation(shader.oglProgramObj, name);
}

int RenderDeviceGL::getShaderSamplerLoc(uint32_t shaderID, const char* name)
{
    RDIShader& shader = mShaders.getRef(shaderID);
    return glGetUniformLocation(shader.oglProgramObj, name);
}

void RenderDeviceGL::setShaderConst(int loc, ShaderConstType type, float* values, uint32_t count)
{
    switch(type) {
    case Float:
        glUniform1fv(loc, count, values);
        break;
    case Float2:
        glUniform2fv(loc, count, values);
        break;
    case Float3:
        glUniform3fv(loc, count, values);
        break;
    case Float4:
        glUniform4fv(loc, count, values);
        break;
    case Float44:
        glUniformMatrix4fv(loc, count, false, values);
        break;
    case Float33:
        glUniformMatrix3fv(loc, count, false, values);
        break;
    }
}

void RenderDeviceGL::setShaderSampler(int loc, uint32_t texUnit)
{
    glUniform1i(loc, (int)texUnit);
}

const char* RenderDeviceGL::getDefaultVSCode()
{
    return defaultShaderVS;
}

const char* RenderDeviceGL::getDefaultFSCode()
{
    return defaultShaderFS;
}

uint32_t RenderDeviceGL::getCurrentShader() const
{
    return mCurShaderID;
}

void RenderDeviceGL::beginRendering()
{
    // Get the currently bound frame buffer object.
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &mDefaultFBO);

    mCurVertexLayout = 1;                     mNewVertexLayout = 0;
    mCurIndexBuffer = 1;                      mNewIndexBuffer = 0;
    mCurRasterState.hash = 0xFFFFFFFFu;       mNewRasterState.hash = 0u;
    mCurBlendState.hash = 0xFFFFFFFFu;        mNewBlendState.hash = 0u;
    mCurDepthStencilState.hash = 0xFFFFFFFFu; mCurDepthStencilState.hash = 0u;

    // for (uint32_t i = 0; i < 16; ++i) setTexture(i, 0, 0);

    setColorWriteMask(true);
    // mPendingMask = 0xFFFFFFFFu;
    mVertexBufUpdated = true;
    commitStates();

    // Bind buffers
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
}

void RenderDeviceGL::finishRendering()
{
    // Nothing to do
}

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

void RenderDeviceGL::destroyBuffer(uint32_t buffer)
{
    if (buffer == 0) return;

    RDIBuffer& buf = mBuffers.getRef(buffer);
    glDeleteBuffers(1, &buf.glObj);

    mBufferMemory -= buf.size;
    mBuffers.remove(buffer);
}

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

uint32_t RenderDeviceGL::getBufferMemory() const
{
    return mBufferMemory;
}

uint32_t RenderDeviceGL::createRenderBuffer(uint32_t width, uint32_t height,
    TextureFormat format, bool depth, uint32_t numColBufs, uint32_t samples)
{
    if ((format == RGBA16F || format == RGBA32F) && !mTexFloatSupported) {
        return 0;
    }

    if (numColBufs > static_cast<unsigned int>(mMaxColBuffers)) {
        return 0;
    }

    uint32_t maxSamples = 0;
    if (mRTMultiSampling) {
        GLint value;
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &value);
        maxSamples = static_cast<uint32_t>(value);
    }
    if (samples > maxSamples) {
        samples = maxSamples;
        Log::warning("GPU does not support desired level of multisampling quality for render "
                     "target");
    }

    RDIRenderBuffer rb;
    rb.width = width;
    rb.height = height;
    rb.samples = samples;

    // Create framebuffers
    glGenFramebuffersEXT(1, &rb.fbo);
    if (samples > 0) glGenFramebuffersEXT(1, &rb.fboMS);

    if (numColBufs > 0) {
        // Attach color buffers
        for (uint32_t i = 0; i < numColBufs; ++i) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fbo);

            // Create a color texturee
            uint32_t texObj = createTexture(Tex2D, rb.width, rb.height, 1, format, false, false,
                false);
            uploadTextureData(texObj, 0, 0, nullptr);
            rb.colTexs[i] = texObj;

            // Attach the texture
            auto& tex = mTextures.getRef(texObj);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,
                GL_TEXTURE_2D, tex.glObj, 0);

            if (samples > 0) {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fboMS);

                // Create a multisampled renderbuffer
                glGenRenderbuffersEXT(1, &rb.colBufs[i]);
                glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb.colBufs[i]);
                glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, rb.samples,
                                                    tex.glFmt, rb.width, rb.height);

                // Attach the renderbuffer
                glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,
                                             GL_RENDERBUFFER_EXT, rb.colBufs[i]);
            }
        }

        static uint32_t buffers[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT,
                                     GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT};
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fbo);
        glDrawBuffers(numColBufs, buffers);

        if (samples > 0) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fboMS);
            glDrawBuffers(numColBufs, buffers);
        }
    }
    else {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fbo);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        if (samples > 0) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fboMS);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }
    }

    if (depth) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fbo);

        // Create a depth texture
        uint32_t texObj = createTexture(Tex2D, rb.width, rb.height, 1, DEPTH, false, false, false);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        uploadTextureData(texObj, 0, 0, nullptr);
        rb.depthTex = texObj;

        // Attach texture
        auto& tex = mTextures.getRef(texObj);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D,
                                  tex.glObj, 0);

        if (samples > 0) {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fboMS);

            // Create a multisampled render buffer
            glGenRenderbuffersEXT(1, &rb.depthBuf);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb.depthBuf);
            glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, rb.samples, mDepthFormat,
                                                rb.width, rb.height);

            // Attach render buffer
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                                         GL_RENDERBUFFER_EXT, rb.depthBuf);
        }
    }

    uint32_t rbObj = mRenderBuffers.add(rb);

    // Check if FBO is cmplete
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fbo);
    uint32_t status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
    bool valid = (status == GL_FRAMEBUFFER_COMPLETE_EXT);

    if (samples > 0) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fboMS);
        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

        if (status != GL_FRAMEBUFFER_COMPLETE_EXT) valid = false;
    }

    if (!valid) {
        destroyRenderBuffer(rbObj);
        return 0;
    }

    return rbObj;
}

void RenderDeviceGL::destroyRenderBuffer(uint32_t rbObj)
{
    auto& rb = mRenderBuffers.getRef(rbObj);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);

    if (rb.depthTex != 0) destroyTexture(rb.depthTex);
    if (rb.depthBuf != 0) glDeleteRenderbuffersEXT(1, &rb.depthBuf);
    rb.depthTex = rb.depthBuf = 0;

    for (uint32_t i = 0; i < RDIRenderBuffer::MaxColorAttachmentCount; ++i) {
        if (rb.colTexs[i] != 0) destroyTexture(rb.colTexs[i]);
        if (rb.colBufs[i] != 0) glDeleteRenderbuffersEXT(1, &rb.colBufs[i]);
        rb.colTexs[i] = rb.colBufs[i] = 0;
    }

    if (rb.fbo != 0) glDeleteFramebuffersEXT(1, &rb.fbo);
    if (rb.fboMS != 0) glDeleteFramebuffersEXT(1, &rb.fboMS);
    rb.fbo = rb.fboMS = 0;

    mRenderBuffers.remove(rbObj);
}

uint32_t RenderDeviceGL::getRenderBufferTexture(uint32_t rbObj, uint32_t bufIndex)
{
    auto& rb = mRenderBuffers.getRef(rbObj);

    if (bufIndex < RDIRenderBuffer::MaxColorAttachmentCount) {
        return rb.colTexs[bufIndex];
    }
    else if (bufIndex == 32) {
        return rb.depthTex;
    }
    else {
        return 0;
    }
}

void RenderDeviceGL::setRenderBuffer(uint32_t rbObj)
{
    // Resolve the renderbuffer if necessary
    if (mCurRenderBuffer != 0) resolveRenderBuffer(rbObj);

    // Set the current renderbuffer
    mCurRenderBuffer = rbObj;

    if (rbObj == 0) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
        if (mDefaultFBO == 0) glDrawBuffer(mOutputBufferIndex ? GL_BACK_RIGHT : GL_BACK_LEFT);

        mFbWidth  = mVpX + mVpWidth;
        mFbHeight = mVpY + mVpHeight;
        glDisable(GL_MULTISAMPLE);
    }
    else {
        // Unbind all textures to make sure that no FBO is bound anymore
        for (uint32_t i = 0; i < 16; ++i) setTexture(i, 0, 0);
        commitStates(Textures);

        auto& rb = mRenderBuffers.getRef(rbObj);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fboMS ? rb.fboMS : rb.fbo);

        mFbWidth  = rb.width;
        mFbHeight = rb.height;

        if (rb.fboMS) {
            glEnable(GL_MULTISAMPLE);
        }
        else {
            glDisable(GL_MULTISAMPLE);
        }
    }
}

void RenderDeviceGL::getRenderBufferSize(uint32_t rbObj, int* width, int* height)
{
    if (rbObj) {
        auto& rb = mRenderBuffers.getRef(rbObj);
        if (width)  *width  = rb.width;
        if (height) *height = rb.height;
    }
    else {
        if (width)  *width  = mVpWidth;
        if (height) *height = mVpHeight;
    }
}

bool RenderDeviceGL::getRenderBufferData(uint32_t rbObj, int bufIndex, int* width, int* height,
    int* compCount, void* dataBuffer, int bufferSize)
{
    int x, y, w, h;
    int format = GL_RGBA;
    int type = GL_FLOAT;
    beginRendering();
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    if (!rbObj) {
        if (bufIndex != 32 && bufIndex != 0) return false;
        if (width)  *width  = mVpWidth;
        if (height) *height = mVpHeight;

        x = mVpX;
        y = mVpY;
        w = mVpWidth;
        h = mVpHeight;

        // format = GL_BGRA;
        // type   = GL_UNSIGNED_BYTE;
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
        if (bufIndex != 32) glReadBuffer(GL_BACK_LEFT);
    }
    else {
        resolveRenderBuffer(rbObj);
        auto& rb = mRenderBuffers.getRef(rbObj);

        if (bufIndex == 32 && rb.depthTex == 0) return false;

        if (
            bufIndex != 32 && (rb.colTexs[bufIndex] ||
            static_cast<unsigned int>(bufIndex) >= RDIRenderBuffer::MaxColorAttachmentCount)
        ) {
            return false;
        }

        if (width)  *width = rb.width;
        if (height) *height = rb.height;

        x = 0;
        y = 0;
        w = rb.width;
        h = rb.height;

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, rb.fbo);
        if (bufIndex != 32) glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + bufIndex);
    }

    if (bufIndex == 32) {
        format = GL_DEPTH_COMPONENT;
        type = GL_UNSIGNED_SHORT; // GL_UNSIGNED_FLOAT
    }

    int comps = (bufIndex == 32 ? 1 : 4);
    if (compCount) *compCount = comps;

    bool retVal = false;
    if (dataBuffer && bufferSize >= w * h * comps * (type == GL_UNSIGNED_SHORT ? 2 : 1)) {
        glReadPixels(x, y, w, h, format, type, dataBuffer);
        retVal = true;
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mDefaultFBO);
    return retVal;
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

void RenderDeviceGL::setIndexBuffer(uint32_t bufObj, IndexFormat format)
{
    mIndexFormat = toIndexFormat[format];
    mNewIndexBuffer = bufObj;
    mPendingMask |= IndexBuffer;
}

void RenderDeviceGL::setVertexBuffer(uint32_t slot, uint32_t vbObj, uint32_t offset,
    uint32_t stride)
{
    auto& vbSlot = mVertBufSlots[slot];
    if (vbSlot.vbObj == vbObj && vbSlot.offset == offset && vbSlot.stride == stride) return;

    mVertexBufUpdated = true;
    vbSlot = {vbObj, offset, stride};
    mPendingMask |= VertexLayouts;
}

void RenderDeviceGL::setVertexLayout(uint32_t vlObj)
{
    mNewVertexLayout = vlObj;
}

void RenderDeviceGL::setTexture(uint32_t slot, uint32_t texObj, uint16_t samplerState)
{
    mTexSlots[slot] = {texObj, samplerState};
    mPendingMask |= Textures;
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
        bool* rtms, bool* occQuery, bool* timerQuery) const
{
    if (maxTexUnits)   *maxTexUnits   = mMaxTextureUnits;
    if (maxTexSize)    *maxTexSize    = mMaxTextureSize;
    if (maxCubTexSize) *maxCubTexSize = mMaxCubeTextureSize;
    if (maxColBufs)    *maxColBufs    = mMaxColBuffers;
    if (dxt)           *dxt           = mDXTSupported;
    if (pvrtci)        *pvrtci        = mPVRTCISupported;
    if (etc1)          *etc1          = mTexETC1Supported;
    if (texFloat)      *texFloat      = mTexFloatSupported;
    if (texDepth)      *texDepth      = mTexDepthSupported;
    if (texSS)         *texSS         = mTexShadowSamplers;
    if (tex3D)         *tex3D         = mTex3DSupported;
    if (texNPOT)       *texNPOT       = mTexNPOTSupported;
    if (texSRGB)       *texSRGB       = mTexSRGBSupported;
    if (rtms)          *rtms          = mRTMultiSampling;
    if (occQuery)      *occQuery      = mOccQuerySupported;
    if (timerQuery)    *timerQuery    = mTimerQuerySupported;
}

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

bool RenderDeviceGL::applyVertexLayout()
{
    if (mNewVertexLayout != 0) {
        if (mCurShaderID == 0) return false;

        uint32_t newVertexAttribMask {0u};

        RDIShader& shader           = mShaders.getRef(mCurShaderID);
        RDIInputLayout& inputLayout = shader.inputLayouts[mNewVertexLayout-1];

        if (!inputLayout.valid) return false;

        // Set vertex attrib pointers
        auto& vl = mVertexLayouts[mNewVertexLayout-1];
        for (uint32_t i = 0; i < vl.numAttribs; ++i) {
            int8_t attribIndex = inputLayout.attribIndices[i];
            if (attribIndex >= 0) {
                VertexLayoutAttrib& attrib = vl.attribs[i];
                const auto& vbSlot = mVertBufSlots[attrib.vbSlot];

                GLenum format = toVertexFormat[attrib.format];
                glBindBuffer(GL_ARRAY_BUFFER, mBuffers.getRef(vbSlot.vbObj).glObj);
                glVertexAttribPointer(
                    attribIndex, attrib.size, format, format == GL_UNSIGNED_BYTE,
                    vbSlot.stride, (char*)0 + vbSlot.offset + attrib.offset
                );

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
    glTexParameteri(
        target, GL_TEXTURE_MIN_FILTER, tex.hasMips ? minFiltersMips[filter] : magFilters[filter]
    );

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

void RenderDeviceGL::applyRenderStates()
{
    // Rasterizer state
    if (mNewRasterState.hash != mCurRasterState.hash) {
        if (mNewRasterState.fillMode == Solid) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

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

void RenderDeviceGL::resolveRenderBuffer(uint32_t rbObj)
{
    auto& rb = mRenderBuffers.getRef(rbObj);

    if (rb.fboMS == 0) return;

    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, rb.fboMS);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, rb.fbo);

    bool depthResolved {false};
    for (uint32_t i = 0; i < RDIRenderBuffer::MaxColorAttachmentCount; ++i) {
        if (rb.colBufs[i] != 0) {
            glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i);

            int mask = GL_COLOR_BUFFER_BIT;
            if (!depthResolved && rb.depthBuf != 0) {
                mask |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
                depthResolved = true;
            }

            glBlitFramebufferEXT(
                0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height, mask, GL_NEAREST
            );
        }
    }

    if (!depthResolved && rb.depthBuf != 0) {
        glReadBuffer(GL_NONE);
        glDrawBuffer(GL_NONE);
        glBlitFramebufferEXT(
            0, 0, rb.width, rb.height, 0, 0, rb.width, rb.height,
            GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST
        );
    }

    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, mDefaultFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, mDefaultFBO);
}

#endif
