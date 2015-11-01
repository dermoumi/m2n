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
#include <mutex>

//==========================================================
// Render device capabilities struct
//==========================================================
struct DeviceCaps
{
    bool texFloat;
    bool texNPOT;
    bool rtMultisampling;
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
    virtual void clear(const float* color) = 0;
    virtual void initStates() = 0;
    virtual void resetStates() = 0;

    // Buffers
    virtual void beginRendering() = 0;
    virtual void finishRendering() = 0;
    virtual uint32_t createVertexBuffer(uint32_t size, const void* data) = 0;
    virtual uint32_t createIndexBuffer(uint32_t size, const void* data) = 0;

    const DeviceCaps& getCapabilities();

protected:
    DeviceCaps mCaps;
};

//==============================================================================
