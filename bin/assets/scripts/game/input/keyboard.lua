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

local Input = require 'game.input'
local Keyboard = require 'window.keyboard'
local Window = require 'window'

local KeyboardInput = Input:subclass 'game.keyboard.input'

function KeyboardInput:initialize(keyMap)
    self:setMap(keyMap or {})

    self.moveX, self.moveY, self.lookX, self.lookY = 0, 0, 0, 0
    self.lookThreshold = 0.66
end

function KeyboardInput:setMap(keyMap)
    self.keyMap = keyMap
    for key, button in pairs(keyMap) do
        if button == 'shift' then
            self.shiftKey = key
            break
        end
    end

    return self
end

function KeyboardInput:setLookThreshold(threshold)
    self.lookThreshold = threshold

    return self
end

function KeyboardInput:handleMoving(button, cb)
    local moved = true
    if button == 'moveup' then
        self.moveY = Keyboard.down(self.shiftKey) and -1 or -0.5
    elseif button == 'movedown' then
        self.moveY = Keyboard.down(self.shiftKey) and 1 or 0.5
    elseif button == 'moveleft' then
        self.moveX = Keyboard.down(self.shiftKey) and -1 or -0.5
    elseif button == 'moveright' then
        self.moveX = Keyboard.down(self.shiftKey) and 1 or 0.5
    else
        moved = false
    end

    if moved then
        cb('stickmoved', 'move', self.moveX, self.moveY)
        return true
    else
        return false
    end
end

function KeyboardInput:calcLookValue(value)
    value = 2*value - 1
    local sign = value < 0 and -1 or 1
    return math.max(0, (value*value - self.lookThreshold) / (1 - self.lookThreshold)) * sign
end

function KeyboardInput:process(cb, e, a, b, c, d)
    if e == 'keydown' then
        local button = self.keyMap[a]
        if button then
            self:handleMoving(button, cb)
            cb('buttondown', button)
        end
    elseif e == 'keyup' then
        local button = self.keyMap[a]
        if button then
            self:handleMoving(button, cb)
            cb('buttonup', button)
        end
    elseif e == 'mousemotion' then
        local w, h = Window.size()
        self.lookX, self.lookY = self:calcLookValue(a/w), self:calcLookValue(b/h)
        cb('stickmoved', 'view', self.lookX, self.lookY)
    end

    Input.process(self, cb, e, a, b, c, d)
end

function KeyboardInput:down(button)
    for key, btn in ipairs(self.keyMap) do
        if btn == button and Keyboard.down(key) then return true end
    end
    return false
end

function KeyboardInput:position(stick)
    if stick == 'move' then
        return self.moveX, self.moveY
    elseif stick == 'look' then
        return self.lookX, self.lookY
    else
        return 0, 0
    end
end

return KeyboardInput