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
function SceneLoad:load(colR, colG, colB, colA, worker, callback)
    if not colB or not colA then
        worker = colR
        callback = colG
        self.colR, self.colG, self.colB, self.colA = 0, 0, 0, 255
    else
        self.colR, self.colG, self.colB, self.colA = colR, colG, colB, colA
    end

    if not callback then
        self.worker = require('game.worker'):new()
        callback = worker
    else
        self.worker = worker
    end

    if type(callback) == 'function' then
        self.callback = callback
    else
        self.worker:addScene(callback)
        self.callback = function()
            Scene.push(callback)
        end
    end

    self.camera = require('nx.camera2d'):new(0, 0, 1280, 720)
end

------------------------------------------------------------
function SceneLoad:update(dt)
    local loaded, failed, total = self.worker:progress()

    if loaded + failed == total then
        Scene.back()
        self.callback()
    end
end

------------------------------------------------------------
function SceneLoad:render()
    self.camera:clear(self.colR, self.colG, self.colB, self.colA)
end

------------------------------------------------------------
return SceneLoad