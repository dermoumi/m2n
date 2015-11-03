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
#include "glcontext.hpp"

#include <SDL2/SDL.h>

#include <memory>
#include <mutex>

//----------------------------------------------------------
// Include from exports
//----------------------------------------------------------
extern "C"
{
    SDL_Window* nxWindowGet();
}

//----------------------------------------------------------
// Locals
//----------------------------------------------------------
namespace
{
    using GlContextPtr = std::unique_ptr<GlContext>;

    std::mutex windowMutex;
    thread_local GlContextPtr currentContext {nullptr};

    std::mutex internalContextMutex;
    GlContextPtr internalContext {nullptr};
}

//----------------------------------------------------------
GlContext* GlContext::create(unsigned int depth, unsigned int stencil, unsigned int msaa)
{
    if (!internalContext) {
        std::lock_guard<std::mutex> lock(internalContextMutex);
        internalContext = GlContextPtr(new GlContext(0, 0, 0));
    }
    else {
        std::lock_guard<std::mutex> lock(internalContextMutex);
        internalContext->setActive(true);
    }

    auto context = new GlContext(depth, stencil, msaa);
    currentContext = GlContextPtr(context);
    return currentContext.get();
}

//----------------------------------------------------------
GlContext* GlContext::ensure()
{
    if (!currentContext) create();
    return currentContext.get();
}

//----------------------------------------------------------
void GlContext::release()
{
    currentContext = nullptr;
}

//----------------------------------------------------------
GlContext::GlContext(unsigned int depth, unsigned int stencil, unsigned int msaa) :
    mDepth(depth),
    mStencil(stencil),
    mMSAA(msaa)
{
    // Set context attributes    
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,  8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   mDepth);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, mStencil);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, mMSAA ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, mMSAA);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, NX_GL_MAJOR);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, NX_GL_MINOR);

    #ifdef NX_OPENGL_ES
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    #endif

    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    // Create the context
    {
        std::lock_guard<std::mutex> lock(windowMutex);
        mContext = SDL_GL_CreateContext(nxWindowGet());
    }

    // Activate the context
    setActive(true);
}

//----------------------------------------------------------
GlContext::~GlContext()
{
    if (!mContext) return;

    setActive(false);
    SDL_GL_DeleteContext(static_cast<SDL_GLContext>(mContext));
}

//----------------------------------------------------------
bool GlContext::setActive(bool active)
{
    if (!mContext) return false;

    if (active) {
        std::lock_guard<std::mutex> lock(windowMutex);
        return SDL_GL_MakeCurrent(nxWindowGet(), mContext) == 0;
    }
    else {
        std::lock_guard<std::mutex> lock(windowMutex);
        return SDL_GL_MakeCurrent(nxWindowGet(), nullptr) == 0;
    }
}

//----------------------------------------------------------
void GlContext::display()
{
    ensure();

    std::lock_guard<std::mutex> lock(windowMutex);
    SDL_GL_SwapWindow(nxWindowGet());
}

//----------------------------------------------------------
bool GlContext::isVSyncEnabled() const
{
    return SDL_GL_GetSwapInterval() > 0;
}

//----------------------------------------------------------
void GlContext::setVSyncEnabled(bool enable)
{
    if (enable) {
        // Attempt to enable late-swap tearing
        if (SDL_GL_SetSwapInterval(-1) == 0) return;

        // Late-swap tearing failed
        SDL_GL_SetSwapInterval(1);
    }
    else {
        SDL_GL_SetSwapInterval(0);
    }
}

//----------------------------------------------------------
void GlContext::getSettings(unsigned int* depth, unsigned int* stencil, unsigned int* msaa) const
{
    if (depth)   *depth   = mDepth;
    if (stencil) *stencil = mStencil;
    if (msaa)    *msaa    = mMSAA;
}

//==============================================================================
