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
function SceneLoad:initialize(a, b, ...)
    local settings

    if type(a) == 'string' then
        settings = {}
        self.nextScene = require(a):new(b, ...)
    elseif type(a) == 'table' and a.isSubclassOf then
        settings = {}
        self.nextScene = a
    else
        settings = a or {}
        self.nextScene = type(b) == 'string' and require(b):new(...) or b
    end

    self.colR = settings.r or 0
    self.colG = settings.g or 0
    self.colB = settings.b or 0
    self.colA = settings.a or 255
end

------------------------------------------------------------
function SceneLoad:load()
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
    self.camera:fillFsQuad(self.colR, self.colG, self.colB, self.colA)
end

------------------------------------------------------------
function SceneLoad:check()
    local loaded, failed, total = self.worker:progress()

    if loaded + failed == total then
        self.nextScene._preloaded = true
        Scene.back()
        Scene.push(self.nextScene)
    end
end

------------------------------------------------------------
function SceneLoad:updateParent()
    return true
end

------------------------------------------------------------
function SceneLoad:renderParent()
    return true
end

------------------------------------------------------------
function SceneLoad:processParent()
    return false
end

------------------------------------------------------------
return SceneLoad