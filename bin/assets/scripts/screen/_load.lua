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

local Text     = require 'graphics.text'
local GameFont = require 'game.font'
local Cache    = require 'game.cache'
local Screen   = require 'screen'

local ScreenLoad = Screen:subclass('screen._load')

function ScreenLoad:initialize(screen)
    self.nextScreen = screen

    local params = screen.__preloadParams or {}

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

function ScreenLoad:entered()
    Cache.prepare()

    self.currentPercent = 0

    self.text = Text:new()
        :setString(self.message, self.currentPercent)
        :setFont(GameFont)
        :setSize(20)
        :setColor(self.messageColR, self.messageColG, self.messageColB, self.messageColA)

    if self:check() then
        print('done?')
        return
    end

    local lastScreen = Screen.lastScreen()
    self.opaque = not lastSCene or lastScreen:isLoading() or lastScreen:isTransitioning()

    self:performTransition()
end

function ScreenLoad:update(dt)
    Cache.iteration()

    -- Calculate fading percent if there's any ongoing fading
    if self:isOpening() then
        self.fadePercent = self.__transTime / self:transitionDuration()
    else
        self.fadePercent = 1
    end

    self:check()

    local w, h = self:view():size()
    self.text:setPosition(30, h - 50)
        :setColor(
            self.messageColR, self.messageColG, self.messageColB,
            self.messageColA * self.fadePercent
        )
end

function ScreenLoad:render()
    -- Draw overlay quad only if there's a screen *currently* rendering behind
    if self.opaque then
        self:view():clear(self.colR, self.colG, self.colB)
    else
        self:view():fillFsQuad(self.colR, self.colG, self.colB, self.colA * self.fadePercent)
    end

    self:view():draw(self.text)
end

function ScreenLoad:renderTransition(time, isOpening)
    if not isOpening then
        Screen.renderTransition(self, time, isOpening)
    end
end

function ScreenLoad:transitionColor()
    return self.colR, self.colG, self.colB, self.colA
end

function ScreenLoad:check()
    local total, loaded, failed = Cache.progress()

    local percent = total ~= 0 and math.floor(100 * loaded / total + 0.5) or 100
    if self.currentPercent <= percent then
        self.currentPercent = math.min(self.currentPercent + 1, percent)
        self.text:setString(self.message, self.currentPercent)
    end

    if loaded >= total then
        if self:isLoading() then
            Screen.replace(self.nextScreen)
        else
            self:performTransition(Screen.replace, self.nextScreen)
        end

        -- self.nextScreen:performTransition()
        return true
    end
end

function ScreenLoad:updateParent()
    return true
end

function ScreenLoad:renderParent()
    return not self.opaque
end

return ScreenLoad
