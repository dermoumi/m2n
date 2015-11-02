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

//==========================================================
// Container for different objects used by the device
//==========================================================
template <class T>
class RDIObjects
{
public:
    uint32_t add(const T& obj)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        
        if (mFreeList.empty()) {
            mObjects.push_back(obj);
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

        if (handle <= 0 || handle > mObjects.size()) return invalidObj;
        std::lock_guard<std::mutex> lock(mMutex);
        
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
    virtual void drawIndexed(RDIPrimType primType, uint32_t firstIndex, uint32_t indexCount,
        uint32_t firstVert, uint32_t vertCount) = 0;

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

    const DeviceCaps& getCapabilities() const;

protected:
    DeviceCaps mCaps;
};

//==============================================================================
