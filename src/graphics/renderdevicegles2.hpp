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
#pragma once
#include "../config.hpp"

#if defined(NX_OPENGL_ES)
//------------------------------------------------------------------------------
#include "renderdevice.hpp"

#include <atomic>

//==========================================================
// OpenGL implementation of RenderDevice
//==========================================================
class RenderDeviceGLES2 : public RenderDevice
{
public:
    bool initialize();

    void initStates();
    void resetStates();
    bool commitStates(uint32_t filter = 0xFFFFFFFFu);

    // Drawcalls and clears
    void clear(const float* color);
    void draw(RDIPrimType primType, uint32_t firstVert, uint32_t vertCount);
    void drawIndexed(RDIPrimType primType, uint32_t firstIndex, uint32_t indexCount);

    // Vertex layouts
    uint32_t registerVertexLayout(uint32_t numAttribs, const VertexLayoutAttrib* attribs);

    // Textures
    uint32_t createTexture(TextureType::Type type, int width, int height, unsigned int depth,
        TextureFormat::Type format, bool hasMips, bool genMips, bool sRGB);
    void uploadTextureData(uint32_t texObj, int slice, int mipLevel, const void* pixels);
    void destroyTexture(uint32_t texObj);
    bool getTextureData(uint32_t texObj, int slice, int mipLevel, void* buffer);
    uint32_t getTextureMemory() const;

    // Shaders
    uint32_t createShader(const char* vertexShaderSrc, const char* fragmentShaderSrc);
    void destroyShader(uint32_t shaderID);
    void bindShader(uint32_t shaderID);
    const std::string& getShaderLog();
    int getShaderConstLoc(uint32_t shaderID, const char* name);
    int getShaderSamplerLoc(uint32_t shaderID, const char* name);
    void setShaderConst(int loc, RDIShaderConstType type, float* values, uint32_t count = 1);
    void setShaderSampler(int loc, uint32_t texUnit);
    const char* getDefaultVSCode();
    const char* getDefaultFSCode();

    // Buffers
    void beginRendering();
    void finishRendering();
    uint32_t createVertexBuffer(uint32_t size, const void* data);
    uint32_t createIndexBuffer(uint32_t size, const void* data);
    void destroyBuffer(uint32_t buffer);
    bool updateBufferData(uint32_t buffer, uint32_t offset, uint32_t size, const void* data);
    uint32_t getBufferMemory() const;

    // GL States
    void setViewport(int x, int y, int width, int height);
    void setScissorRect(int x, int y, int width, int height);
    void setIndexBuffer(uint32_t bufObj, RDIIndexFormat idxFmt);
    void setVertexBuffer(uint32_t slot, uint32_t vbObj, uint32_t offset, uint32_t stride);
    void setVertexLayout(uint32_t vlObj);
    void setTexture(uint32_t slot, uint32_t texObj, uint16_t samplerState);

    // Render states
    void setColorWriteMask(bool enabled);
    bool getColorWriteMask() const;
    void setFillMode(RDIFillMode fillMode);
    RDIFillMode getFillMode() const;
    void setCullMode(RDICullMode cullMode);
    RDICullMode getCullMode() const;
    void setScissorTest(bool enabled);
    bool getScissorTest() const;
    void setMultisampling(bool enabled);
    bool getMultisampling() const;
    void setAlphaToCoverage(bool enabled);
    bool getAlphaToCoverage() const;
    void setBlendMode(bool enabled, RDIBlendFunc src = BS_BLEND_ZERO,
        RDIBlendFunc dst = BS_BLEND_ZERO);
    bool getBlendMode(RDIBlendFunc& src, RDIBlendFunc& dst) const;
    void setDepthMask(bool enabled);
    bool getDepthMask() const;
    void setDepthTest(bool enabled);
    bool getDepthTest() const;
    void setDepthFunc(RDIDepthFunc depthFunc);
    RDIDepthFunc getDepthFunc() const;

    // Capabilities
    void getCapabilities(unsigned int& maxTexUnits, unsigned int& maxTexSize,
        unsigned int& maxCubTexSize, bool& dxt, bool& pvrtci, bool& etc1, bool& texFloat,
        bool& texDepth, bool& texSS, bool& tex3d, bool& texNPOT, bool& texSRGB, bool& rtms,
        bool& occQuery, bool& timerQuery) const;

private:
    constexpr static uint32_t MaxNumVertexLayouts = 16;

    struct RDIBuffer
    {
        uint32_t type;
        uint32_t glObj;
        uint32_t size;
    };

    struct RDIInputLayout
    {
        bool valid;
        int8_t attribIndices[16];
    };

    struct RDITexture
    {
        uint32_t            glObj;
        uint32_t            glFmt;
        int                 type;
        TextureFormat::Type format;
        int                 width;
        int                 height;
        int                 depth;
        int                 memSize;
        uint32_t            samplerState;
        bool                sRGB;
        bool                hasMips;
        bool                genMips;
    };

    struct RDIShader
    {
        uint32_t oglProgramObj;
        RDIInputLayout inputLayouts[MaxNumVertexLayouts];
    };

    struct RDIVertBufSlot
    {
        uint32_t vbObj;
        uint32_t offset;
        uint32_t stride;
    };

    struct RDITexSlot
    {
        uint32_t texObj;
        uint32_t samplerState;
    };

    struct RDIRasterState
    {
        union
        {
            uint32_t hash;
            struct
            {
                uint32_t fillMode              : 1;
                uint32_t cullMode              : 2;
                uint32_t scissorEnable         : 1;
                uint32_t multisampleEnable     : 1;
                uint32_t renderTargetWriteMask : 1;
            };
        };
    };

    struct RDIBlendState
    {
        union
        {
            uint32_t hash;
            struct
            {
                uint32_t alphaToCoverageEnable : 1;
                uint32_t blendEnable           : 1;
                uint32_t srcBlendFunc          : 4;
                uint32_t dstBlendFunc          : 4;
            };
        };
    };

    struct RDIDepthStencilState
    {
        union
        {
            uint32_t hash;
            struct
            {
                uint32_t depthWriteMask : 1;
                uint32_t depthEnable    : 1;
                uint32_t depthFunc      : 4;
            };
        };
    };

    enum PendingMask : uint32_t
    {
        PMViewport     = 1 << 0,
        PMIndexBuffer  = 1 << 1,
        PMVertexLayout = 1 << 2,
        PMTextures     = 1 << 3,
        PMScissor      = 1 << 4,
        PMRenderStates = 1 << 5
    };

private:
    uint32_t createShaderProgram(const char* vertexShaderSrc, const char* fragmentShaderSrc);
    bool linkShaderProgram(uint32_t programObj);

    bool applyVertexLayout();
    void applySamplerState(RDITexture& tex);
    void applyRenderStates();

private:
    uint32_t mDepthFormat;
    int mVpX {0}, mVpY {0}, mVpWidth {1}, mVpHeight {1};
    int mScX {0}, mScY {0}, mScWidth {1}, mScHeight {1};
    std::atomic<uint32_t> mBufferMemory  {0u};
    std::atomic<uint32_t> mTextureMemory {0u};

    int mDefaultFBO {0};
    std::atomic<uint32_t>  mNumVertexLayouts{0};
    RDIVertexLayout        mVertexLayouts[MaxNumVertexLayouts];
    RDIObjects<RDIBuffer>  mBuffers;
    RDIObjects<RDIShader>  mShaders;
    RDIObjects<RDITexture> mTextures;

    RDIVertBufSlot       mVertBufSlots[16];
    RDITexSlot           mTexSlots[16];
    RDIRasterState       mCurRasterState,       mNewRasterState;
    RDIBlendState        mCurBlendState,        mNewBlendState;
    RDIDepthStencilState mCurDepthStencilState, mNewDepthStencilState;
    uint32_t mPrevShaderID {0u},    mCurShaderID {0u};
    uint32_t mCurVertexLayout {0u}, mNewVertexLayout {0u};
    uint32_t mCurIndexBuffer {0u},  mNewIndexBuffer {0u};
    uint32_t mIndexFormat {0u};
    uint32_t mActiveVertexAttribsMask {0u};
    uint32_t mPendingMask {0u};

    int mMaxTextureUnits      {0};
    int mMaxTextureSize       {0};
    int mMaxCubeTextureSize   {0};
    bool mDXTSupported        {false};
    bool mPVRTCISupported     {false};
    bool mTexETC1Supported    {false};
    bool mTexFloatSupported   {false};
    bool mTexDepthSupported   {false};
    bool mTexShadowSamplers   {false};
    bool mTex3DSupported      {false};
    bool mTexNPOTSupported    {false};
    bool mTexSRGBSupported    {false};
    bool mRTMultiSampling     {false};
    bool mOccQuerySupported   {false};
    bool mTimerQuerySupported {false};
};

//------------------------------------------------------------------------------
#endif
//==============================================================================
