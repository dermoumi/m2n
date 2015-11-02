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
#include "../system/log.hpp"

#if !defined(NX_OPENGL_ES)
//------------------------------------------------------------------------------
#include "opengl.hpp"

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
    mCaps.texFloat         = glExt::ARB_texture_float;
    mCaps.texNPOT = glExt::ARB_texture_non_power_of_two;
    mCaps.rtMultisampling = glExt::EXT_framebuffer_multisample;

    // Set some default values
    mIndexFormat = GL_UNSIGNED_SHORT;

    // TODO: Find supported depth format (some old ATI cards only support 16 bit depth for FBOs)

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
    // TODO: complete this

    mCurVertexLayout = 1; mNewVertexLayout = 0;
    mCurIndexBuffer = 1;  mNewIndexBuffer = 0;

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
            // TODO
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
void RenderDeviceGL::clear(const float* color)
{
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::registerVertexLayout(uint32_t numAttribs,
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
uint32_t RenderDeviceGL::createShader(const char* vertexShaderSrc, const char* fragmentShaderSrc)
{
    // Compile and link shader
    uint32_t programObj = createShaderProgram(vertexShaderSrc, fragmentShaderSrc);
    if (programObj == 0) return 0;
    if (!linkShaderProgram(programObj)) return 0;


    uint32_t shaderID = mShaders.add({});
    RDIShader& shader = mShaders.getRef(shaderID);
    shader.oglProgramObj = programObj;

    int attribCount;
    glGetProgramiv(programObj, GL_ACTIVE_ATTRIBUTES, &attribCount);

    for (uint32_t i = 0; i < mNumVertexLayouts; ++i) {
        RDIVertexLayout& v1 = mVertexLayouts[i];
        bool allAttribsFound = true;

        for (uint32_t j = 0; j < 16; ++j) {
            shader.inputLayouts[i].attribIndices[j] = -1;
        }

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

        shader.inputLayouts[i].valid = allAttribsFound;
    }

    return shaderID;
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
    return mShaderLog;
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
void RenderDeviceGL::setVertexBuffer(uint32_t, uint32_t, uint32_t, uint32_t)
{
    // TODO
}

//----------------------------------------------------------
void RenderDeviceGL::setVertexLayout(uint32_t vlObj)
{
    mNewVertexLayout = vlObj;
}

//----------------------------------------------------------
void RenderDeviceGL::setTexture(uint32_t, uint32_t, uint16_t)
{
    // TODO
}

//----------------------------------------------------------
uint32_t RenderDeviceGL::createShaderProgram(const char* vertexShaderSrc,
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
bool RenderDeviceGL::linkShaderProgram(uint32_t programObj)
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

//------------------------------------------------------------------------------
bool RenderDeviceGL::applyVertexLayout()
{
    // uint32_t newVertexAttribMask {0u};

    // if (mNewVertexLayout != 0) {
    //     if (mCurShaderID == 0) return false;

    //     RDIVertexLayout& v1         = mVertexLayouts[mNewVertexLayout - 1];
    //     RDIShader& shader           = mShaders.getRef(mCurShaderID);
    //     RDIInputLayout& inputLayout = shader.inputLayout[mNewVertexLayout - 1];

    //     if (!inputLayout.valid) return false;

    //     // Set vertex attrib pointers
    //     for (uint32_t i = 0; i < v1.numAttribs; ++i) {
    //         int8_t attribIndex = inputLayout.attribIndices[i];
    //         if (attribIndex >= 0) {
    //             VertexLayoutAttrib& attrib = v1.attribs[i];
    //             cnst RDIVertBufSlot& vbSlot = mVertB
    //         }
    //     }
    // }

    // TODO
    return true;
}

//------------------------------------------------------------------------------
#endif
//==============================================================================
