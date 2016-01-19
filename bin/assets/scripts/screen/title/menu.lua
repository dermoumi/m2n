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

local Screen = require 'screen'
local Text = require 'graphics.text'
local GameFont = require 'game.font'

local TitleMenu = Screen:subclass 'screen.title.menu'

function TitleMenu:entered()
    local newGame = {
        text = Text:new('Start Game', GameFont, 60),
        callback = function() print('newgame') end
    }

    local settings = {
        text = Text:new('Settings', GameFont, 48),
        callback = function() print('settings') end
    }

    local credits = {
        text = Text:new('Credits', GameFont, 48),
        callback = function() print('credits') end
    }

    local quit = {
        text = Text:new('Quit', GameFont, 48),
        callback = function() require('window').close() end
    }

    self.items = {newGame, settings, credits, quit}
    self.currentItem = 1
    self.lastItem = nil
end

function TitleMenu:update(dt)
    if self.currentItem ~= self.lastItem then
        for i, item in ipairs(self.items) do
            if self.currentItem == i then
                item.text:setColor(255, 86, 72)
            else
                item.text:setColor(255, 255, 255)
            end
        end
        self.lastItem = self.currentItem
    end
end

function TitleMenu:resized(width, height)
    Screen.resized(self, width, height)

    local padding, menuH = math.floor(height/30), 0
    for i, item in ipairs(self.items) do
        menuH = menuH + item.text:size() + padding
    end

    local yOffset = 0
    for i, item in ipairs(self.items) do
        local x, y, w = item.text:bounds()
        item.text:setPosition(
            width/2 - w/2,
            height/2 - menuH/2 + yOffset
        )
        yOffset = yOffset + item.text:size() + padding
    end
end

function TitleMenu:mousemotion(mouseX, mouseY)
    self.currentItem = nil
    for i, item in ipairs(self.items) do
        local x, y, w, h = item.text:bounds(true)
        x, y, w, h = x - 10, y - 10, w + 20, h + 20
        if mouseX >= x and mouseX <= x + w and mouseY >= y and mouseY <= y + h then
            self.currentItem = i
            break
        end
    end
end

function TitleMenu:mousedown(x, y, button)
    if self.currentItem then
        self.items[self.currentItem].callback()
    else
        self:returnBack()
    end
end

function TitleMenu:keydown(scancode)
    if scancode == 'escape' then
        self:returnBack()
    elseif scancode == 'up' then
        if not self.currentItem or self.currentItem == 1 then
            self.currentItem = #self.items
        else
            self.currentItem = self.currentItem - 1
        end
    elseif scancode == 'down' then
        if not self.currentItem or self.currentItem == #self.items then
            self.currentItem = 1
        else
            self.currentItem = self.currentItem + 1
        end
    end
end

function TitleMenu:render()
    -- Do not render anything if not the current screen
    if self ~= Screen.currentScreen() then
        return
    end

    -- Render menu
    for i, item in ipairs(self.items) do
        self:view():draw(item.text)
    end
end

function TitleMenu:processParent()
    return true
end

function TitleMenu:returnBack()
    Screen.back()
end

return TitleMenu