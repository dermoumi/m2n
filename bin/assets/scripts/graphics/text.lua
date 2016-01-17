--[[
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
--]]

local Unicode      = require 'util.unicode'
local Graphics     = require 'graphics'
local VertexBuffer = require 'graphics.vertexbuffer'
local Texture      = require 'graphics.texture'
local Entity2D     = require 'graphics.entity2d'

local Text = Entity2D:subclass 'graphics.text'

local ffi = require 'ffi'
local C = ffi.C

ffi.cdef [[
    typedef struct NxText NxText;

    NxText* nxTextNew();
    void nxTextRelease(NxText*);
    void nxTextSetString(NxText*, const char*);
    void nxTextSetU32String(NxText*, const uint32_t*);
    void nxTextSetFont(NxText*, const void*);
    void nxTextSetCharacterSize(NxText*, uint32_t);
    void nxTextSetStyle(NxText*, uint8_t);
    void nxTextCharacterPosition(const NxText*, uint32_t, float*);
    void nxTextBounds(const NxText*, float*);
    NxVertexBuffer* nxTextNextBuffer(const NxText*, uint32_t*);
]]

local indexPtr = ffi.new('uint32_t[1]')
local vertexHelper = VertexBuffer:allocate()

local toStyle = {
    regular       = 0,
    bold          = 1,
    italic        = 2,
    underlined    = 4,
    strikethrough = 8
}

function Text.static._defaultShader()
    return Graphics.defaultShader(1)
end

function Text.static._vertexLayout()
    return Graphics.vertexLayout(1)
end

function Text:initialize(str, font, charSize)
    Entity2D.initialize(self)

    self._cdata = ffi.gc(C.nxTextNew(), C.nxTextRelease)

    self:setString(str or '')
        :setFont(font)
        :setSize(charSize or 30)

    self._style = 0
end

function Text:release()
    if self._cdata == 0 then return end

    C.nxTextRelease(ffi.gc(self._cdata, nil))
    self._cdata = nil
end

function Text:setString(str, arg, ...)
    if type(str) == 'string' then
        if arg then str = str:format(arg, ...) end

        if self._string == str then return self end
        self._string = str
        self._u32string = nil

        C.nxTextSetString(self._cdata, str)
    else
        -- Make sure it ends with a 0
        if str[#str] ~= 0 then str[#str+1] = 0 end

        if self._u32string == str then return self end
        self._u32string = str
        self._string = nil

        local strPtr = ffi.new('uint32_t[?]', #str, str)
        C.nxTextSetU32String(self._cdata, strPtr)
    end

    return self
end

function Text:setFont(font)
    self._font = font
    if font then C.nxTextSetFont(self._cdata, font._cdata) end

    return self
end

function Text:setSize(size)
    self._charSize = size
    C.nxTextSetCharacterSize(self._cdata, size)

    return self
end

function Text:setStyle(style1, style2, ...)
    self._style = {style1, style2, ...}

    local styleCompound = 0

    if not style2 then
        styleCompound = toStyle[style1]
    else
        local bit = require 'bit'
        for i, style in ipairs(self._style) do
            styleCompound = bit.bor(styleCompound, toStyle[style] or 0)
        end
    end

    C.nxTextSetStyle(self._cdata, styleCompound)

    return self
end

function Text:string(u32)
    if not self._string and not self._u32string then return '' end

    if u32 then
        if not self._u32string then
            self._u32string = Unicode.utf8To32(self._str)
        end
        return self._u32string
    else
        if not self._string then
            self._string = Unicode.utf32To8(self._str)
        end
        return self._string
    end
end

function Text:font()
    return self._font
end

function Text:size()
    return self._charSize
end

function Text:style()
    return unpack(self._style)
end

function Text:characterPosition(index)
    local posPtr = ffi.new('float[2]')
    C.nxTextCharacterPosition(self._cdata, index, posPtr)

    return posPtr[0], posPtr[1]
end

function Text:bounds()
    local boundsPtr = ffi.new('float[4]')
    C.nxTextBounds(self._cdata, boundsPtr)

    return boundsPtr[0], boundsPtr[1], boundsPtr[2], boundsPtr[3]
end

function Text:_render(camera)
    if self._cdata ~= nil and self._font and self._font._cdata ~= nil then
        local shader = self._shader or Text._defaultShader()

        shader:bind()
        shader:setUniform('uProjMat', camera:projection())
        shader:setUniform('uTransMat', self:matrix(true))
        shader:setUniform('uColor', self:color(true, true))
        shader:setSampler('uTexture0', 0)

        repeat
            vertexHelper._cdata = C.nxTextNextBuffer(self._cdata, indexPtr)
            if vertexHelper._cdata == nil then break end

            vertexHelper:bind()
            C.nxRendererSetVertexLayout(Text._vertexLayout())
            
            local texture = self._font:texture(self._charSize, indexPtr[0])
            texture:bind()

            shader:setUniform('uTexSize', texture:size())

            C.nxRendererDraw(4, 0, vertexHelper:count())
        until false

    end
end

return Text
