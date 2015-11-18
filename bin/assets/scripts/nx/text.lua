--[[----------------------------------------------------------------------------
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
--]]----------------------------------------------------------------------------

------------------------------------------------------------
-- FFI C Declarations
------------------------------------------------------------
local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxText NxText;
    typedef struct NxArraybuffer NxArraybuffer;

    NxText* nxTextNew();
    void nxTextRelease(NxText*);
    void nxTextSetString(NxText*, const char*);
    void nxTextSetFont(NxText*, const void*);
    void nxTextSetCharacterSize(NxText*, uint32_t);
    void nxTextSetStyle(NxText*, uint8_t);
    const char* nxTextString(const NxText*);
    const void* nxTextFont(const NxText*);
    uint32_t nxTextCharacterSize(const NxText*);
    uint8_t nxTextStyle(const NxText*);
    void nxTextCharacterPosition(const NxText*, uint32_t, float*);
    void nxTextBounds(const NxText*, float*);
    const NxArraybuffer* nxTextArraybuffer(const NxText*, uint32_t*);
]]

------------------------------------------------------------
local Entity2D = require 'nx.entity2d'
local Text = Entity2D:subclass('nx.text')

local Renderer = require 'nx.renderer'
local Arraybuffer = require'nx.arraybuffer'
local Texture = require 'nx.texture'

local vertCountPtr = ffi.new('uint32_t[1]')

------------------------------------------------------------
function Text.static._fromCData(cdata)
    local text = Text:allocate()

    Entity2D.initialize(text)
    text._cdata = ffi.cast('NxText*', cdata)
    text._font = require('nx._font')._fromCData(nil)
    text._arraybuffer = require('nx.arraybuffer')._fromCData(nil)

    return text
end

------------------------------------------------------------
function Text.static._defaultShader()
    return Renderer.defaultShader(1)
end

------------------------------------------------------------
function Text.static._vertexLayout()
    return Renderer.vertexLayout(1)
end

------------------------------------------------------------
function Text:initialize()
    Entity2D.initialize(self)

    local handle = C.nxTextNew()
    self._cdata = ffi.gc(handle, C.nxTextRelease)

    self._font = require('nx._font')._fromCData(nil)
    self._arraybuffer = require('nx.arraybuffer')._fromCData(nil)
end

------------------------------------------------------------
function Text:release()
    if self._cdata == 0 then return end

    C.nxTextRelease(ffi.gc(self._cdata, nil))
end

------------------------------------------------------------
function Text:setString(str)
    C.nxTextSetString(self._cdata, str)
end

------------------------------------------------------------
function Text:setFont(font)
    self._font._cdata = font._cdata
    C.nxTextSetFont(self._cdata, font._cdata)
end

------------------------------------------------------------
function Text:setCharacterSize(size)
    C.nxTextSetCharacterSize(self._cdata, size)
end

------------------------------------------------------------
function Text:setStyle(style)
    C.nxTextSetStyle(self._cdata, style)
end

------------------------------------------------------------
function Text:string()
    return ffi.string(C.nxTextString(self._cdata))
end

------------------------------------------------------------
function Text:font()
    self._font._cdata = C.nxTextFont(self._cdata)
    return self._font
end

------------------------------------------------------------
function Text:characterSize()
    return C.nxTextCharacterSize(self._cdata)
end

------------------------------------------------------------
function Text:style()
    return C.nxTextStyle(self._cdata)
end

------------------------------------------------------------
function Text:characterPosition(index)
    local posPtr = ffi.new('float[2]')
    C.nxTextCharacterPosition(self._cdata, index, posPtr)
    return posPtr[0], posPtr[1]
end

------------------------------------------------------------
function Text:bounds()
    local boundsPtr = ffi.new('float[4]')
    C.nxTextBounds(self._cdata, boundsPtr)
    return boundsPtr[0], boundsPtr[1], boundsPtr[2], boundsPtr[3]
end

------------------------------------------------------------
function Text:_render(camera, transMat, r, g, b, a)
    transMat:combine(self:matrix())

    if self._font and self._font._cdata ~= nil then
        r = r or 255
        g = g or 255
        b = b or 255
        a = a or 255
        
        local texture = self._font:texture(self:characterSize())
        texture:bind(0)
        local texW, texH = texture:size()

        local shader = self._shader or Text._defaultShader()
        shader:bind()
        shader:setUniform('uTransMat', transMat)
        shader:setUniform('uColor', r / 255, g / 255, b / 255, a / 255)
        shader:setUniform('uTexSize', texW, texH)
        shader:setSampler('uTexture', 0)

        local vertices = Arraybuffer._fromCData(C.nxTextArraybuffer(self._cdata, vertCountPtr))
        Arraybuffer.setVertexbuffer(vertices, 0, 0, 16)
        Arraybuffer.setIndexbuffer(nil)
        C.nxRendererSetVertexLayout(Text._vertexLayout())

        C.nxRendererDraw(0, 0, vertCountPtr[0])
    end
end

------------------------------------------------------------
return Text