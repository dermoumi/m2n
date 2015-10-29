#pragma once

class GlContext
{
public:
    static GlContext* create(unsigned int depth = 0u, unsigned int stencil = 0, unsigned int msaa = 0u);
    static GlContext* ensure();
    static void release();
    
    ~GlContext();
    bool setActive(bool active);
    void display();
    bool isVSyncEnabled() const;
    void setVSyncEnabled(bool enable);
    void getSettings(unsigned int* depth, unsigned int* stencil, unsigned int* msaa) const;
    void getGLVersion(unsigned int* major, unsigned int* minor) const;

private:
    GlContext(unsigned int depth, unsigned int stencil, unsigned int msaa);

private:
    mutable void* mContext {nullptr};
    unsigned int  mDepth;
    unsigned int  mStencil;
    unsigned int  mMSAA;
    unsigned int  mGLMajor;
    unsigned int  mGLMinor;
};