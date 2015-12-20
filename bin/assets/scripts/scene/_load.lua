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

local GameFont = require 'game.font'
local Text     = require 'nx.text'
local Scene    = require 'scene'

local SceneLoad = Scene:subclass('scene._load')

------------------------------------------------------------
function SceneLoad:initialize(scene)
    self.nextScene = scene
    self.worker    = scene.__worker

    local params = scene.__preloadParams or {}

    -- Set background color
    self.colR = params.r or 0
    self.colG = params.g or 0
    self.colB = params.b or 0
    self.colA = params.a or 255

    -- Calculate lighting value of the given color, and decide whether the text should be
    -- light or dark
    local r, g, b, a = self.colR, self.colG, self.colB, self.colA
    if (math.max(r, g, b) + math.min(r, g, b)) < 408 then
        r, g, b, a = 255, 255, 255, 255
    else
        r, g, b, a = 0, 0, 0, 255
    end

    -- Set the color
    self.messageColR = params.messageColR or r
    self.messageColG = params.messageColG or g
    self.messageColB = params.messageColB or b
    self.messageColA = params.messageColA or a
    self.message = params.message or 'LOADING %i%%'
end

------------------------------------------------------------
function SceneLoad:load()
    self.worker:start()

    self.text = Text:new()
        :setString(self.message, 0)
        :setFont(GameFont)
        :setSize(20)
        :setColor(self.messageColR, self.messageColG, self.messageColB, self.messageColA)

    if self:check() then return end

    self.opaque = not self.parent or self.parent:isLoading() or self.parent:isTransitioning()

    self:performTransition()
end

------------------------------------------------------------
function SceneLoad:update(dt)
    self:check()

    self.fadePercent = 1

    -- Calculate fading percent if there's any ongoing fading
    if self:isTransitioning() and not self.__transitionFadingIn then
        self.fadePercent = self.__transitionTime / self:transitionDuration()
    end

    local w, h = self:view():size()
    self.text:setPosition(30, h - 50)
        :setColor(
            self.messageColR, self.messageColG, self.messageColB,
            self.messageColA * self.fadePercent
        )
end

------------------------------------------------------------
function SceneLoad:render()
    -- Draw overlay quad only if there's a scene *currently* rendering behind
    if self.opaque then
        self:view():clear(self.colR, self.colG, self.colB)
    else    
        self:view():fillFsQuad(self.colR, self.colG, self.colB, self.colA * self.fadePercent)
    end

    self:view():draw(self.text)
end

------------------------------------------------------------
function SceneLoad:renderTransition(time, isFadingIn)
    if not self.parent or isFadingIn then
        Scene.renderTransition(self, time, isFadingIn)
    end
end

------------------------------------------------------------
function SceneLoad:transitionColor()
    return self.colR, self.colG, self.colB, self.colA
end

------------------------------------------------------------
function SceneLoad:check()
    local loaded, failed, total = self.worker:progress()

    local percent = total ~= 0 and math.floor(100 * (loaded + failed) / total + 0.5) or 1
    self.text:setString(self.message, percent)

    if loaded + failed == total then
        if self:isLoading() then
            Scene.replace(self.nextScene)
        else
            self:performTransition(Scene.replace, self.nextScene)
        end

        self.nextScene:performTransition()
        return true
    end
end

------------------------------------------------------------
function SceneLoad:updateParent()
    return true
end

------------------------------------------------------------
function SceneLoad:renderParent()
    return not self.opaque
end

------------------------------------------------------------
return SceneLoad