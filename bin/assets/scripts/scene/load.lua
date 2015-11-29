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

    local r, g, b, a = self.colR, self.colG, self.colB, self.colA
    if (math.max(r, g, b) + math.min(r, g, b)) < 408 then
        r, g, b, a = 255, 255, 255, 255
    else
        r, g, b, a = 0, 0, 0, 255
    end

    self.messageColR = settings.messageColR or r
    self.messageColG = settings.messageColG or g
    self.messageColB = settings.messageColB or b
    self.messageColA = settings.messageColA or a
    self.message = settings.message or 'LOADING %i%%'
end

------------------------------------------------------------
function SceneLoad:load()
    self.worker = require('game.worker'):new()
    self.nextScene:preload(self.worker)

    self.lastPercent = 0

    self.text = require('nx.text'):new()
    self.text:setFont(require('game.font'))
    self.text:setCharacterSize(20)
    self.text:setString(self.message:format(0))
    self.text:setColor(self.messageColR, self.messageColG, self.messageColB, self.messageColA)

    self.camera = require('nx.camera2d'):new(0, 0, require('nx.window').size())

    if self:check() then return end

    self.parentLoading = self.parent and self.parent._isLoading

    self:performTransition(self.camera)
end

------------------------------------------------------------
function SceneLoad:update(dt)
    self:check()

    self.fadePercent = 1

    -- Calculate fading percent if there's any ongoing fading
    if self._transitionTime and not self._transitionFadingIn then
        self.fadePercent = self._transitionTime / self:transitionDuration()
    end

    local w, h = self.camera:size()
    self.text:setPosition(30, h - 50)
    self.text:setColor(
        self.messageColR, self.messageColG, self.messageColB, self.messageColA * self.fadePercent
    )
end

------------------------------------------------------------
function SceneLoad:render()
    local fadePercent = 1

    -- Draw overlay quad only if there's a scene *currently* rendering behind
    if self.parent and not self.parentLoading then
        self.camera:fillFsQuad(self.colR, self.colG, self.colB, self.colA * self.fadePercent)
    else
        self.camera:clear(self.colR, self.colG, self.colB)
    end

    self.camera:draw(self.text)
end

------------------------------------------------------------
function SceneLoad:renderTransition(camera, time, duration, fadingIn)
    if not self.parent or self._transitionFadingIn then
        Scene.renderTransition(self, camera, time, duration, fadingIn)
    end
end

------------------------------------------------------------
function SceneLoad:transitionColor()
    return self.colR, self.colG, self.colB, self.colA
end

------------------------------------------------------------
function SceneLoad:check()
    local loaded, failed, total = self.worker:progress()

    if loaded + failed == total then
        local function callback()
            self.nextScene._preloaded = true
            Scene.back()
            Scene.push(self.nextScene)
        end

        if self._isLoading then
            callback()
            return true
        else
            self:performTransition(self.camera, callback)
        end
    end

    local percent = total ~= 0 and math.floor(100 * (loaded + failed) / total + 0.5) or 1
    if self.lastPercent ~= percent then
        self.text:setString(self.message:format(percent))
        self.lastPercent = percent
    end
end

------------------------------------------------------------
function SceneLoad:onResize(w, h)
    self.camera:reset(0, 0, w, h)
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
