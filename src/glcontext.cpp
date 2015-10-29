#include "glcontext.hpp"
#include "config.hpp"
#include <SDL2/SDL.h>
#include <memory>
#include <mutex>

#if defined(NX_OPENGL_ES)
    #include <SDL2/SDL_opengles2.h>
#else
    #include <SDL2/SDL_opengl.h>
#endif

#if !defined(GL_MAJOR_VERSION)
    #define GL_MAJOR_VERSION 0x821B
#endif

#if !defined(GL_MINOR_VERSION)
    #define GL_MINOR_VERSION 0x821C
#endif

#if !defined(GL_MULTISAMPLE)
    #define GL_MULTISAMPLE 0x809D
#endif

extern "C"
{
    // Exported by src/exports/nxwindow.cpp
    SDL_Window* nxWindowGet();
}

namespace
{
    using GlContextPtr = std::unique_ptr<GlContext>;

    std::mutex windowMutex;
    thread_local GlContextPtr currentContext {nullptr};
}

void GlContext::create(unsigned int depth, unsigned int stencil, unsigned int msaa)
{
    currentContext = GlContextPtr(new GlContext(depth, stencil, msaa));
}

void GlContext::ensure()
{
    if (!currentContext) create();
}

void GlContext::release()
{
    currentContext = nullptr;
}

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

    // Retrieve the actual context version
    int majorVersion {0};
    int minorVersion {0};
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

    if (glGetError() != GL_INVALID_ENUM) {
        mGLMajor = static_cast<unsigned int>(majorVersion);
        mGLMinor = static_cast<unsigned int>(minorVersion);
    }
    else {
        // Try the "old" way
        const GLubyte* version {glGetString(GL_VERSION)};
        if (version) {
            // The returned string starts with major.minor (it's a standard)
            mGLMajor = version[0] - '0';
            mGLMinor = version[2] - '0';
        }
        else {
            // Can't get the version number, assume 1.1
            mGLMajor = 1;
            mGLMinor = 1;
        }
    }

    // Enable antialiasing
    if (mMSAA > 0) glEnable(GL_MULTISAMPLE);
}

GlContext::~GlContext()
{
    if (!mContext) return;

    setActive(false);
    SDL_GL_DeleteContext(static_cast<SDL_GLContext>(mContext));
}

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

void GlContext::display()
{
    ensure();

    std::lock_guard<std::mutex> lock(windowMutex);
    SDL_GL_SwapWindow(nxWindowGet());
}

bool GlContext::isVSyncEnabled() const
{
    return SDL_GL_GetSwapInterval() > 0;
}

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

void GlContext::getSettings(unsigned int* depth, unsigned int* stencil, unsigned int* msaa) const
{
    if (depth)   *depth   = mDepth;
    if (stencil) *stencil = mStencil;
    if (msaa)    *msaa    = mMSAA;
}

void GlContext::getGLVersion(unsigned int* major, unsigned int* minor) const
{
    if (major) *major = mGLMajor;
    if (minor) *minor = mGLMinor;
}