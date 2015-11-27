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
local Scene = require 'scene'
local SceneLoad = Scene:subclass('scene.load')

------------------------------------------------------------
function SceneLoad:initialize(colR, colG, colB, colA, nextScene, ...)
    if not colG then
        self.colR, self.colG, self.colB, self.colA = 0, 0, 0, 255

        if type(colR) == 'string' then
            self.nextScene = require(colR):new(colR, colB, colA, nextScene, ...)
        end
    else
        self.colR, self.colG, self.colB, self.colA = colR, colG, colB, colA or 255
        if type(nextScene) == 'string' then
            self.nextScene = require(nextScene):new(...)
        end
    end
end

------------------------------------------------------------
function SceneLoad:load(colR, colG, colB, colA, nextScene, ...)
    self.worker = require('game.worker'):new()
    self.nextScene:preload(self.worker)

    self:check()

    self.camera = require('nx.camera2d'):new(0, 0, 1280, 720)
end

------------------------------------------------------------
function SceneLoad:update(dt)
    self:check()
end

------------------------------------------------------------
function SceneLoad:render()
    require('nx.renderbuffer').bind(nil)
    require('nx.renderer').fillFsQuad(self.colR, self.colG, self.colB, self.colA)
end

------------------------------------------------------------
function SceneLoad:check()
    local loaded, failed, total = self.worker:progress()

    if loaded + failed == total then
        Scene.back()
        Scene.push(self.nextScene)
    end
end

------------------------------------------------------------
function SceneLoad:processParent()
    return false
end

------------------------------------------------------------
return SceneLoad