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
GlContext* GlContext::ensure()
{
    if (!currentContext) {
        {
            std::lock_guard<std::mutex> lock(internalContextMutex);
            if (!internalContext) {
                internalContext = GlContextPtr(new GlContext());
            }

            internalContext->setActive(true);
        }

        currentContext = GlContextPtr(new GlContext());
    }

    currentContext->setActive(true);
    return currentContext.get();
}

//----------------------------------------------------------
void GlContext::release()
{
    currentContext = nullptr;
}

//----------------------------------------------------------
GlContext::GlContext()
{
    // Create the context
    std::lock_guard<std::mutex> lock(windowMutex);
    mContext = SDL_GL_CreateContext(nxWindowGet());
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

//==============================================================================
