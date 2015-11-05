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
    // TODO: complete this

    mCurVertexLayout = 1; mNewVertexLayout = 0;
    mCurIndexBuffer = 1;  mNewIndexBuffer = 0;

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
            // TODO
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
            // TODO
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

    mVertexLayouts[mNumVertexLayouts].numAttribs = numAttribs;
    for (uint32_t i = 0; i < numAttribs; ++i) {
        mVertexLayouts[mNumVertexLayouts].attribs[i] = attribs[i];
    }

    return ++mNumVertexLayouts;
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
        RDIVertexLayout& v1 = mVertexLayouts[i];
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
            for (uint32_t k = 0; k < v1.numAttribs; ++k) {
                if (v1.attribs[j].semanticName == name) {
                    shader.inputLayouts[i].attribIndices[k] = glGetAttribLocation(programObj, name);
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
    return mShaderLog;
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
void RenderDeviceGLES2::setTexture(uint32_t, uint32_t, uint16_t)
{
    // TODO
}

//----------------------------------------------------------
uint32_t RenderDeviceGLES2::createShaderProgram(const char* vertexShaderSrc,
    const char* fragmentShaderSrc)
{
    int infoLogLength {0};
    int charsWritten  {0};
    char* infoLog     {nullptr};
    int status;

    mShaderLog = "";

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
            mShaderLog = mShaderLog + "[Vertex Shader]\n" + infoLog;
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
            mShaderLog = mShaderLog + "[Fragment Shader]\n" + infoLog;
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

    mShaderLog = "";

    glLinkProgram(programObj);
    glGetProgramiv(programObj, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 1) {
        infoLog = new char[infoLogLength];
        glGetProgramInfoLog(programObj, infoLogLength, &charsWritten, infoLog);
        mShaderLog = mShaderLog + "[Linking]\n" + infoLog;
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

//------------------------------------------------------------------------------
#endif
//==============================================================================
