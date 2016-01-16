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
#include "shader.hpp"
#include "texture.hpp"
#include "indexbuffer.hpp"
#include "vertexbuffer.hpp"
#include "renderbuffer.hpp"

#include <cstring>
#include <vector>
#include <string>

// Base class for Render Devices
class RenderDevice
{
public:
    struct VertexLayoutAttrib
    {
        std::string semanticName;
        uint8_t     vbSlot;
        uint8_t     size;
        uint8_t     offset;
        uint8_t     format;
    };

    struct VertexLayout
    {
        uint32_t           numAttribs;
        VertexLayoutAttrib attribs[16];
    };

    enum VertexFormat {
        VtxFloat,
        VtxU8
    };

    enum PrimType {
        Points,
        Lines,
        LineStrip,
        LineLoop,
        Triangles,
        TriangleStrip,
        TriangleFan
    };

    enum ShaderConstType {
        Float,
        Float2,
        Float3,
        Float4,
        Float44,
        Float33
    };

    enum FillMode {
        Solid,
        Wireframe
    };

    enum CullMode {
        Back,
        Front,
        None
    };

    enum BlendFunc {
        Zero,
        One,
        SrcAlpha,
        InvSrcAlpha,
        DstColor
    };

    enum DepthFunc {
        LessEqual,
        Less,
        Equal,
        Greater,
        GreaterEqual,
        Always
    };

    enum PendingMask : uint32_t {
        Viewport      = 1 << 0,
        IndexBuffers  = 1 << 1,
        VertexLayouts = 1 << 2,
        Textures      = 1 << 3,
        Scissor       = 1 << 4,
        RenderStates  = 1 << 5,
        SamplerState  = 1 << 6
    };

    enum ClearFlags : uint32_t {
        ClrColorRT0 = 1 << 0,
        ClrColorRT1 = 1 << 1,
        ClrColorRT2 = 1 << 2,
        ClrColorRT3 = 1 << 3,
        ClrDepth    = 1 << 4
    };

public:
    static RenderDevice& instance();

    virtual ~RenderDevice() = default;
    virtual bool initialize() = 0;

    virtual void initStates() = 0;
    virtual void resetStates() = 0;
    virtual bool commitStates(uint32_t filter = 0xFFFFFFFFu) = 0;
    virtual void beginRendering() = 0;
    virtual void finishRendering() = 0;

    // Drawcalls and clears
    virtual void clear(uint32_t flags, const float* color, float depth) = 0;
    virtual void draw(PrimType primType, uint32_t firstVert, uint32_t vertCount) = 0;
    virtual void drawIndexed(PrimType primType, uint32_t firstIndex, uint32_t indexCount) = 0;

    // Vertex layouts
    virtual uint32_t registerVertexLayout(uint8_t numAttribs,
        const VertexLayoutAttrib* attribs) = 0;

    // Vertex buffers
    virtual VertexBuffer* newVertexBuffer() = 0;
    virtual uint32_t usedVertexBufferMemory() const = 0;
    virtual void bind(VertexBuffer* buffer, uint8_t slot, uint32_t offset) = 0;

    // Index buffers
    virtual IndexBuffer* newIndexBuffer() = 0;
    virtual uint32_t usedIndexBufferMemory() const = 0;
    virtual void bind(IndexBuffer* buffer) = 0;

    // Textures
    virtual Texture* newTexture() = 0;
    virtual void bind(const Texture* texture, uint8_t slot) = 0;
    virtual uint32_t usedTextureMemory() const = 0;

    // Shaders
    virtual Shader* newShader() = 0;
    virtual void bind(Shader* shader) = 0;
    virtual const std::string& getShaderLog() = 0;
    virtual const char* getDefaultVSCode() = 0;
    virtual const char* getDefaultFSCode() = 0;
    virtual Shader* getCurrentShader() const = 0;

    // Renderbuffers
    virtual RenderBuffer* newRenderBuffer() = 0;
    virtual void bind(RenderBuffer* buffer) = 0;

    // GL States
    virtual void setViewport(int x, int y, int width, int height) = 0;
    virtual void setScissorRect(int x, int y, int width, int height) = 0;
    virtual void setVertexLayout(uint32_t vlObj) = 0;

    // Render states
    virtual void setColorWriteMask(bool enabled) = 0;
    virtual bool getColorWriteMask() const = 0;
    virtual void setFillMode(FillMode fillMode) = 0;
    virtual FillMode getFillMode() const = 0;
    virtual void setCullMode(CullMode cullMode) = 0;
    virtual CullMode getCullMode() const = 0;
    virtual void setScissorTest(bool enabled) = 0;
    virtual bool getScissorTest() const = 0;
    virtual void setMultisampling(bool enabled) = 0;
    virtual bool getMultisampling() const = 0;
    virtual void setAlphaToCoverage(bool enabled) = 0;
    virtual bool getAlphaToCoverage() const = 0;
    virtual void setBlendMode(bool enabled, BlendFunc src = Zero, BlendFunc dst = Zero) = 0;
    virtual bool getBlendMode(BlendFunc& src, BlendFunc& dst) const = 0;
    virtual void setDepthMask(bool enabled) = 0;
    virtual bool getDepthMask() const = 0;
    virtual void setDepthTest(bool enabled) = 0;
    virtual bool getDepthTest() const = 0;
    virtual void setDepthFunc(DepthFunc depthFunc) = 0;
    virtual DepthFunc getDepthFunc() const = 0;

    virtual void sync() = 0;

    // Capabilities
    virtual void getCapabilities(uint32_t* maxTexUnits, uint32_t* maxTexSize,
        uint32_t* maxCubeTexSize, uint32_t* maxColBufs, bool* dxt, bool* pvrtci, bool* etc1,
        bool* texFloat, bool* texDepth, bool* texSS, bool* tex3d, bool* texNPOT, bool* texSRGB,
        bool* rtms, bool* occQuery, bool* timerQuery, bool* multithreading) const = 0;
};
