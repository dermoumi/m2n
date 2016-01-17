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

#pragma once
#include "../config.hpp"

#if !defined(NX_OPENGL_ES)
#include "renderdevice.hpp"

#include <atomic>

// OpenGL implementation of RenderDevice
class RenderDeviceGL : public RenderDevice
{
public:
    bool initialize();

    void initStates();
    void resetStates();
    bool commitStates(uint32_t filter = 0xFFFFFFFFu);

    // Drawcalls and clears
    void clear(uint32_t flags, const float* color, float depth);
    void draw(PrimType primType, uint32_t firstVert, uint32_t vertCount);
    void drawIndexed(PrimType primType, uint32_t firstIndex, uint32_t indexCount);
    void beginRendering();
    void finishRendering();

    // Vertex layouts
    uint32_t registerVertexLayout(uint8_t numAttribs, const VertexLayoutAttrib* attribs);

    // Vertex buffers
    VertexBuffer* newVertexBuffer();
    uint32_t usedVertexBufferMemory() const;
    void bind(VertexBuffer* buffer, uint8_t slot, uint32_t offset);

    // Index buffers
    IndexBuffer* newIndexBuffer();
    uint32_t usedIndexBufferMemory() const;
    void bind(IndexBuffer* buffer);

    // Textures
    Texture* newTexture();
    void bind(const Texture* texture, uint8_t slot);
    uint32_t usedTextureMemory() const;

    // Shaders
    Shader* newShader();
    void bind(Shader* shader);
    const std::string& getShaderLog();
    const char* getDefaultVSCode();
    const char* getDefaultFSCode();
    Shader* getCurrentShader() const;

    // Renderbuffers
    RenderBuffer* newRenderBuffer();
    void bind(RenderBuffer* buffer);

    // GL States
    void setViewport(int x, int y, int width, int height);
    void setScissorRect(int x, int y, int width, int height);
    void setVertexLayout(uint32_t vlObj);

    // Render states
    void setColorWriteMask(bool enabled);
    bool getColorWriteMask() const;
    void setFillMode(FillMode fillMode);
    FillMode getFillMode() const;
    void setCullMode(CullMode cullMode);
    CullMode getCullMode() const;
    void setScissorTest(bool enabled);
    bool getScissorTest() const;
    void setMultisampling(bool enabled);
    bool getMultisampling() const;
    void setAlphaToCoverage(bool enabled);
    bool getAlphaToCoverage() const;
    void setBlendMode(bool enabled, BlendFunc src = Zero, BlendFunc dst = Zero);
    bool getBlendMode(BlendFunc& src, BlendFunc& dst) const;
    void setDepthMask(bool enabled);
    bool getDepthMask() const;
    void setDepthTest(bool enabled);
    bool getDepthTest() const;
    void setDepthFunc(DepthFunc depthFunc);
    DepthFunc getDepthFunc() const;

    void sync();

    // Capabilities
    void getCapabilities(uint32_t* maxTexUnits, uint32_t* maxTexSize,
        uint32_t* maxCubTexSize, uint32_t* maxColBufs, bool* dxt, bool* pvrtci, bool* etc1,
        bool* texFloat, bool* texDepth, bool* texSS, bool* tex3d, bool* texNPOT, bool* texSRGB,
        bool* rtms, bool* occQuery, bool* timerQuery, bool* multithreading) const;

private:
    constexpr static uint32_t MaxNumVertexLayouts = 16;

    struct RDIInputLayout
    {
        bool valid;
        int8_t attribIndices[16];
    };

    struct RDIVertBufSlot
    {
        VertexBuffer* vbObj;
        uint32_t offset;
    };

    struct RDIRasterState
    {
        union
        {
            uint32_t hash;
            struct
            {
                uint8_t fillMode              : 1;
                uint8_t cullMode              : 2;
                uint8_t scissorEnable         : 1;
                uint8_t multisampleEnable     : 1;
                uint8_t renderTargetWriteMask : 1;
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
                uint8_t alphaToCoverageEnable : 1;
                uint8_t blendEnable           : 1;
                uint8_t srcBlendFunc          : 4;
                uint8_t dstBlendFunc          : 4;
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
                uint8_t depthWriteMask : 1;
                uint8_t depthEnable    : 1;
                uint8_t depthFunc      : 4;
            };
        };
    };

    class ShaderGL : public Shader
    {
    public:
        ShaderGL(RenderDeviceGL* device);
        ~ShaderGL();

        bool load(const char* vertexShader, const char* fragmentShader);
        void setUniform(int location, uint8_t type, float* data, uint32_t count = 1);
        void setSampler(int location, uint8_t unit);

        int uniformLocation(const char* name) const;
        int samplerLocation(const char* name) const;

    private:
        friend class RenderDeviceGL;

        RenderDeviceGL* mDevice;
        uint32_t        mHandle {0u};
        RDIInputLayout  mInputLayouts[MaxNumVertexLayouts];
    };

    class VertexBufferGL : public VertexBuffer
    {
    public:
        VertexBufferGL(RenderDeviceGL* device);
        ~VertexBufferGL();

        bool load(void* data, uint32_t size, uint32_t stride);
        bool update(void* data, uint32_t size, uint32_t offset);

        uint32_t size() const;
        uint32_t stride() const;

    private:
        friend class RenderDeviceGL;

        void release();

        RenderDeviceGL* mDevice;
        uint32_t        mHandle {0u};
        uint32_t        mSize {0u};
        uint32_t        mStride {0u};
    };

    class IndexBufferGL : public IndexBuffer
    {
    public:
        IndexBufferGL(RenderDeviceGL* device);
        ~IndexBufferGL();

        bool load(void* data, uint32_t size, Format format);
        bool update(void* data, uint32_t size, uint32_t offset);

        uint32_t size() const;
        Format format() const;

    private:
        friend class RenderDeviceGL;

        void release();

        RenderDeviceGL* mDevice;
        uint32_t        mHandle {0u};
        uint32_t        mSize {0u};
        Format          mFormat {_16};
    };

    class RenderBufferGL;
    class TextureGL : Texture
    {
    public:
        TextureGL(RenderDeviceGL* device);
        ~TextureGL();

        bool create(Type type, Format format, uint16_t width, uint16_t height,
            bool hasMips, bool mipMaps, bool srgb);
        void setData(const void* buffer, uint8_t slice, uint8_t level);
        void setSubData(const void* buffer, uint16_t x, uint16_t y, uint16_t width,
            uint16_t height, uint8_t slice, uint8_t level);
        bool data(void* buffer, uint8_t slice, uint8_t level) const;
        uint32_t bufferSize() const;

        uint16_t width() const;
        uint16_t height() const;

        void setFilter(Filter filter);
        void setAnisotropyLevel(Anisotropy aniso);
        void setRepeating(uint32_t repeating);
        void setLessOrEqual(bool enable);

        Filter filter() const;
        Anisotropy anisotropyLevel() const;
        uint32_t repeating() const;
        bool lessOrEqual() const;

        bool flipCoords() const;
        Type type() const;
        Format format() const;

    private:
        friend class RenderDeviceGL;
        friend class RenderBufferGL;

        void release();
        void applyState() const;

        RenderDeviceGL* mDevice;
        uint32_t mHandle {0u};
        uint32_t mGlFormat {0u};
        uint32_t mGlType {0u};
        Type mType {_2D};
        Format mFormat {Unknown};
        uint16_t mWidth {0u};
        uint16_t mHeight {0u};
        bool mSrgb {false};
        bool mHasMips {true};
        bool mMipMaps {true};
        uint32_t mState {0u};
        uint32_t mMemSize {0u};
        RenderBufferGL* mRenderBuffer {nullptr};
    };

    struct TextureSlot
    {
        const TextureGL* texture {nullptr};
        uint32_t         state {0u};
    };

    class RenderBufferGL : public RenderBuffer
    {
    public:
        RenderBufferGL(RenderDeviceGL* device);
        ~RenderBufferGL();

        bool create(Texture::Format format, uint16_t width, uint16_t height, bool depth,
            uint8_t colBufCount, uint8_t samples);
        Texture* texture(uint8_t index);

        uint16_t width() const;
        uint16_t height() const;
        Texture::Format format() const;
        // TODO: virtual void* data() const;

        static constexpr uint32_t MaxColorAttachmentCount = 4;

    private:
        friend class RenderDeviceGL;

        void resolve();
        void release();

        RenderDeviceGL* mDevice;
        uint32_t mFbo {0u};
        uint32_t mFboMS {0u};
        uint16_t mWidth {0u};
        uint16_t mHeight {0u};
        uint8_t mSamples {0u};
        Texture::Format mFormat;

        std::shared_ptr<TextureGL> mDepthTex {0u};
        std::shared_ptr<TextureGL> mColTexs[MaxColorAttachmentCount];
        uint32_t mDepthBuf {0u};
        uint32_t mDepthBufMS {0u};
        uint32_t mColBufs[MaxColorAttachmentCount];
    };

private:
    bool applyVertexLayout();
    void applyRenderStates();

private:
    uint32_t mDepthFormat;
    int mVpX {0}, mVpY {0}, mVpWidth {1}, mVpHeight {1};
    int mScX {0}, mScY {0}, mScWidth {1}, mScHeight {1};
    std::atomic<uint32_t> mVertexBufferMemory {0u};
    std::atomic<uint32_t> mIndexBufferMemory {0u};
    std::atomic<uint32_t> mTextureMemory {0u};
    RenderBufferGL* mCurRenderBuffer {nullptr};
    int mDefaultFBO        {0};
    int mFbWidth           {0};
    int mFbHeight          {0};
    int mOutputBufferIndex {0};

    std::atomic<uint32_t>  mNumVertexLayouts {0u};
    VertexLayout           mVertexLayouts[MaxNumVertexLayouts];

    RDIVertBufSlot       mVertBufSlots[16];
    TextureSlot          mTexSlots[16];
    RDIRasterState       mCurRasterState,       mNewRasterState;
    RDIBlendState        mCurBlendState,        mNewBlendState;
    RDIDepthStencilState mCurDepthStencilState, mNewDepthStencilState;
    Shader *mPrevShader {nullptr}, *mCurShader {nullptr};
    IndexBuffer *mCurIndexBuffer {nullptr}, *mNewIndexBuffer {nullptr};
    uint32_t mCurVertexLayout {0u}, mNewVertexLayout {0u};
    uint32_t mIndexFormat {0u};
    uint32_t mActiveVertexAttribsMask {0u};
    uint32_t mPendingMask {0u};
    bool mVertexBufUpdated {true};

    int mMaxTextureUnits      {0};
    int mMaxTextureSize       {0};
    int mMaxCubeTextureSize   {0};
    int mMaxColBuffers        {0};
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

#endif
