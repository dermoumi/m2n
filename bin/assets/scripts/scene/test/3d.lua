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

local Scene = require 'scene'

local SceneTest3D = Scene:subclass('scene.test.3d')

------------------------------------------------------------
function SceneTest3D:initialize()
    self:worker():addFile('nx.image', 'assets/pasrien.png')
end

------------------------------------------------------------
function SceneTest3D:load()
    self.camera = require('nx.camera3d'):new(70, 16/9, -1, 1)

    self.texture = require('nx.texture2d'):new()
        :load(self:cache('assets/pasrien.png'))

    self.sprite = require('nx.sprite'):new(self.texture)
        :setScale(1/512, 1/512)
        :setPosition(-1/2, -1/2)
end

------------------------------------------------------------
function SceneTest3D:render()
    require('nx.renderer').setCullMode('none')
    self.camera:clear(200, 200, 200)

    self.camera:draw(self.sprite)
end

------------------------------------------------------------
function SceneTest3D:onKeyDown(scancode, keyCode, repeated)
    if scancode == 'down' then
        self.camera:translate(0, 0, -.1)
    elseif scancode == 'up' then
        self.camera:translate(0, 0, .1)
    elseif scancode == 'f1' then
        self:transition(Scene.back)
    end
end

------------------------------------------------------------
function SceneTest3D:updateParent()
    return true
end

------------------------------------------------------------
return SceneTest3D