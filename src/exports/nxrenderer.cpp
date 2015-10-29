#include "../config.hpp"

#if defined(NX_OPENGL_ES)
    #include <SDL2/SDL_opengles2.h>
#else
    #include <SDL2/SDL_opengl.h>
#endif

extern "C"
{
    NX_EXPORT void nxRendererClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        glClearColor(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
