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

#include <cstring>
#include <vector>
#include <string>
#include <mutex>

//==========================================================
// Structures and enums
//==========================================================
struct DeviceCaps
{
    bool texFloat;
    bool texNPOT;
    bool rtMultisampling;
};

//----------------------------------------------------------
struct VertexLayoutAttrib
{
    std::string semanticName;
    uint32_t    vbSlot;
    uint32_t    size;
    uint32_t    offset;
};

//----------------------------------------------------------
struct RDIVertexLayout
{
    uint32_t           numAttribs;
    VertexLayoutAttrib attribs[16];
};

//----------------------------------------------------------
enum RDIIndexFormat : int
{
    IDXFMT_16,
    IDXFMT_32
};

//----------------------------------------------------------
enum RDIPrimType : int
{
    PRIM_TRIANGLES,
    PRIM_TRISTRIP
};

//----------------------------------------------------------
enum RDIShaderConstType
{
    CONST_FLOAT,
    CONST_FLOAT2,
    CONST_FLOAT3,
    CONST_FLOAT4,
    CONST_FLOAT44,
    CONST_FLOAT33
};

//----------------------------------------------------------
namespace TextureType
{
    enum Type : int
    {
        Tex2D,
        Tex3D,
        TexCube
    };
}

//----------------------------------------------------------
namespace TextureFormat
{
    enum Type : int
    {
        Unknown,
        RGBA8,
        DXT1,
        DXT3,
        DXT5,
        RGBA16F,
        RGBA32F,
        DEPTH,
        Count
    };
}

//----------------------------------------------------------
namespace SamplerState
{
    enum Type
    {
        FilterBilinear   = 0x0,
        FilterTrilinear  = 0x0001,
        FilterPoint      = 0x0002,
        Aniso1           = 0x0,
        Aniso2           = 0x0004,
        Aniso4           = 0x0008,
        Aniso8           = 0x0010,
        Aniso16          = 0x0020,
        AddrUClamp       = 0x0,
        AddrUWrap        = 0x0040,
        AddrUClampCol    = 0x0080,
        AddrVClamp       = 0x0,
        AddrVWrap        = 0x0100,
        ADdrVClampCol    = 0x0200,
        AddrWClamp       = 0x0,
        AddrWWrap        = 0x0400,
        AddrWClampCol    = 0x0800,
        AddrClamp        = AddrUClamp | AddrVClamp | AddrWClamp,
        AddrWrap         = AddrUWrap | AddrVWrap | AddrWWrap,
        AddrClampCol     = AddrUClampCol | ADdrVClampCol | AddrWClampCol,
        CompLEqual       = 0x1000
    };

    constexpr uint32_t FilterStart = 0;
    constexpr uint32_t FilterMask = FilterBilinear | FilterTrilinear | FilterPoint;
    constexpr uint32_t AnisoStart = 2;
    constexpr uint32_t AnisoMask = Aniso1 | Aniso2 | Aniso4 | Aniso8 | Aniso16;
    constexpr uint32_t AddrUStart = 6;
    constexpr uint32_t AddrUMask = AddrUClamp | AddrUWrap | AddrWClampCol;
    constexpr uint32_t AddrVStart = 8;
    constexpr uint32_t AddrVMask = AddrVClamp | AddrVWrap | ADdrVClampCol;
    constexpr uint32_t AddrWStart = 10;
    constexpr uint32_t AddrWMask = AddrWClamp | AddrWWrap | AddrWClampCol;
    constexpr uint32_t AddrStart = 6;
    constexpr uint32_t AddrMask = AddrClamp | AddrWrap | AddrClampCol;
}

//----------------------------------------------------------
enum RDIFillMode
{
    RS_FILL_SOLID = 0,
    RS_FILL_WIREFRAME = 1
};

//----------------------------------------------------------
enum RDICullMode
{
    RS_CULL_BACK = 0,
    RS_CULL_FRONT,
    RS_CULL_NONE
};

//----------------------------------------------------------
enum RDIBlendFunc
{
    BS_BLEND_ZERO = 0,
    BS_BLEND_ONE,
    BS_BLEND_SRC_ALPHA,
    BS_BLEND_INV_SRC_ALPHA,
    BS_BLEND_DEST_COLOR
};

//----------------------------------------------------------
enum RDIDepthFunc
{
    DSS_DEPTHFUNC_LESS_EQUAL = 0,
    DSS_DEPTHFUNC_LESS,
    DSS_DEPTHFUNC_EQUAL,
    DSS_DEPTHFUNC_GREATER,
    DSS_DEPTHFUNC_GREATER_EQUAL,
    DSS_DEPTHFUNC_ALWAYS
};

//==========================================================
// Container for different objects used by the device
//==========================================================
template <class T>
class RDIObjects
{
public:
    uint32_t add(const T& obj, bool copy = false)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        
        if (mFreeList.empty()) {
            if (copy) {
                mObjects.emplace_back();
                memcpy(&mObjects.back(), &obj, sizeof(T));
            }
            else {
                mObjects.emplace_back(obj);
            }

            return static_cast<uint32_t>(mObjects.size());
        }
        else {
            auto index = mFreeList.back();
            mFreeList.pop_back();
            mObjects[index] = obj;
            return index + 1;
        }
    }

    void remove(uint32_t handle)
    {
        if (handle <= 0 || handle > mObjects.size()) return;
        std::lock_guard<std::mutex> lock(mMutex);

        mObjects[handle - 1] = T(); // Destruct and replace with an invalid object
        mFreeList.push_back(handle - 1);
    }

    T& getRef(uint32_t handle)
    {
        thread_local T invalidObj;

        std::lock_guard<std::mutex> lock(mMutex);
        if (handle <= 0 || handle > mObjects.size()) return invalidObj;
        
        return mObjects[handle - 1];
    }

private:
    friend class RenderDevice;
    std::vector<T>        mObjects;
    std::vector<uint32_t> mFreeList;
    std::mutex            mMutex;
};

//==========================================================
// Base class for Render Devices
//==========================================================
class RenderDevice
{
public:
    virtual ~RenderDevice() = default;
    virtual bool initialize() = 0;

    virtual void initStates() = 0;
    virtual void resetStates() = 0;
    virtual bool commitStates(uint32_t filter = 0xFFFFFFFFu) = 0;

    // Drawcalls and clears
    virtual void clear(const float* color) = 0;
    virtual void draw(RDIPrimType primType, uint32_t firstVert, uint32_t vertCount) = 0;
    virtual void drawIndexed(RDIPrimType primType, uint32_t firstIndex, uint32_t indexCount) = 0;

    // Vertex layouts
    virtual uint32_t registerVertexLayout(uint32_t numAttribs,
        const VertexLayoutAttrib* attribs) = 0;

    // Buffers
    virtual void beginRendering() = 0;
    virtual void finishRendering() = 0;
    virtual uint32_t createVertexBuffer(uint32_t size, const void* data) = 0;
    virtual uint32_t createIndexBuffer(uint32_t size, const void* data) = 0;
    virtual void destroyBuffer(uint32_t buffer) = 0;
    virtual bool updateBufferData(uint32_t buffer, uint32_t offset, uint32_t size,
        const void* data) = 0;
    virtual uint32_t getBufferMemory() const = 0;

    // Textures
    uint32_t calcTextureSize(TextureFormat::Type format, int width, int height, int depth);
    virtual uint32_t createTexture(TextureType::Type type, int width, int height,
        unsigned int depth, TextureFormat::Type format, bool hasMips, bool genMips, bool sRGB) = 0;
    virtual void uploadTextureData(uint32_t texObj, int slice, int mipLevel,
        const void* pixels) = 0;
    virtual void uploadTextureSubData(uint32_t texObj, int slice, int mipLevel, unsigned int x,
        unsigned int y, unsigned int z, unsigned int width, unsigned int height, unsigned int depth,
        const void* pixels) = 0;
    virtual void destroyTexture(uint32_t texObj) = 0;
    virtual bool getTextureData(uint32_t texObj, int slice, int mipLevel, void* buffer) = 0;
    virtual uint32_t getTextureMemory() const = 0;

    // Shaders
    virtual uint32_t createShader(const char* vertexShaderSrc, const char* fragmentShaderSrc) = 0;
    virtual void destroyShader(uint32_t shaderID) = 0;
    virtual void bindShader(uint32_t shaderID) = 0;
    virtual const std::string& getShaderLog() = 0;
    virtual int getShaderConstLoc(uint32_t shaderID, const char* name) = 0;
    virtual int getShaderSamplerLoc(uint32_t shaderID, const char* name) = 0;
    virtual void setShaderConst(int loc, RDIShaderConstType type, float* values,
        uint32_t count = 1) = 0;
    virtual void setShaderSampler(int loc, uint32_t texUnit) = 0;
    virtual const char* getDefaultVSCode() = 0;
    virtual const char* getDefaultFSCode() = 0;

    // GL States
    virtual void setViewport(int x, int y, int width, int height) = 0;
    virtual void setScissorRect(int x, int y, int width, int height) = 0;
    virtual void setIndexBuffer(uint32_t bufObj, RDIIndexFormat idxFmt) = 0;
    virtual void setVertexBuffer(uint32_t slot, uint32_t vbObj, uint32_t offset,
        uint32_t stride) = 0;
    virtual void setVertexLayout(uint32_t vlObj) = 0;
    virtual void setTexture(uint32_t slot, uint32_t texObj, uint16_t samplerState) = 0;

    // Render states
    virtual void setColorWriteMask(bool enabled) = 0;
    virtual bool getColorWriteMask() const = 0;
    virtual void setFillMode(RDIFillMode fillMode) = 0;
    virtual RDIFillMode getFillMode() const = 0;
    virtual void setCullMode(RDICullMode cullMode) = 0;
    virtual RDICullMode getCullMode() const = 0;
    virtual void setScissorTest(bool enabled) = 0;
    virtual bool getScissorTest() const = 0;
    virtual void setMultisampling(bool enabled) = 0;
    virtual bool getMultisampling() const = 0;
    virtual void setAlphaToCoverage(bool enabled) = 0;
    virtual bool getAlphaToCoverage() const = 0;
    virtual void setBlendMode(bool enabled, RDIBlendFunc src = BS_BLEND_ZERO,
        RDIBlendFunc dst = BS_BLEND_ZERO) = 0;
    virtual bool getBlendMode(RDIBlendFunc& src, RDIBlendFunc& dst) const = 0;
    virtual void setDepthMask(bool enabled) = 0;
    virtual bool getDepthMask() const = 0;
    virtual void setDepthTest(bool enabled) = 0;
    virtual bool getDepthTest() const = 0;
    virtual void setDepthFunc(RDIDepthFunc depthFunc) = 0;
    virtual RDIDepthFunc getDepthFunc() const = 0;

    // Capabilities
    virtual void getCapabilities(unsigned int* maxTexUnits, unsigned int* maxTexSize,
        unsigned int* maxCubTexSize, bool* dxt, bool* pvrtci, bool* etc1, bool* texFloat,
        bool* texDepth, bool* texSS, bool* tex3d, bool* texNPOT, bool* texSRGB, bool* rtms,
        bool* occQuery, bool* timerQuery) const = 0;
};

//==============================================================================
