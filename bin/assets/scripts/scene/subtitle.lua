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
-- A test scene to be pushed over title
------------------------------------------------------------
local class = require 'nx.class'
local Scene = require 'scene'
local SceneSubtitle = class('scene.subtitle', Scene)

------------------------------------------------------------
local Mouse = require 'nx.mouse'
local Renderer = require 'nx.renderer'
local Thread = require 'nx.thread'

------------------------------------------------------------
function SceneSubtitle:load()
    Renderer.testInit()

    self._processParent = true
end

------------------------------------------------------------
function SceneSubtitle:release()
    Renderer.testRelease()
    print('bye?')
end

------------------------------------------------------------
function SceneSubtitle:render()
    Renderer.testRender()
end

------------------------------------------------------------
function SceneSubtitle:onKeyDown(scancode)
    if scancode == 'f2' then
        Scene.back()
        return false
    elseif scancode == 'f3' then
        self._processParent = not self._processParent
    elseif scancode == 'space' then
        print(require('nx.window').hasMouseFocus())
    end
end

------------------------------------------------------------
function SceneSubtitle:processParent()
    return self._processParent
end

------------------------------------------------------------
return SceneSubtitle