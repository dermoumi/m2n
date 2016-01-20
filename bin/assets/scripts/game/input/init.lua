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

local class = require 'class'
local Input = class 'game.input'

local inputs = {}

function Input.static.registerInput(input, id)
    inputs[id or 1] = input
end

function Input.static.get(id)
    return inputs[id or 1]
end

function Input.static.reset()
    for i, input in pairs(inputs) do
        input:reset()
    end
end

function Input.static.processEvent(e, a, b, c, d)
    for i, input in pairs(inputs) do
        input:processEvent(e, a, b, c, d)
    end
end

function Input.static.updateState()
    for i, input in pairs(inputs) do
        input:updateState()
    end
end

function Input.static.down(...)
    for i, input in pairs(inputs) do
        if input:down(...) then return true end
    end
    return false
end

function Input.static.pressed(...)
    for i, input in pairs(inputs) do
        if input:pressed(...) then return true end
    end
    return false
end

function Input.static.released(...)
    for i, input in pairs(inputs) do
        if input:released(...) then return true end
    end
    return false
end

function Input:initialize()
    self:reset()
end

function Input:reset()
    self.lastState = {}
    self.state = {}
    self.nextState = {}
end

function Input:updateState()
    self.lastState = self.state
    self.state = {}
    for i in pairs(self.nextState) do
        self.state[i] = true
    end
end

function Input:down(a, b, ...)
    if b then
        for i, key in ipairs{a, b, ...} do
            if self.state[key] then return true end
        end
        return false
    else
        return self.state[a]
    end
end

function Input:pressed(a, b, ...)
    if b then
        for i, key in ipairs{a, b, ...} do
            if self.state[key] and not self.lastState[key] then return true end
        end
        return false
    else
        return self.state[a] and not self.lastState[a]
    end
end

function Input:released(a, b, ...)
    if b then
        for i, key in ipairs{a, b, ...} do
            if not self.state[key] and self.lastState[key] then return true end
        end
        return false
    else
        return not self.state[a] and self.lastState[a]
    end
end

function Input:clicked(id, x, y, w, h)
    return false
end

function Input:moved(id)
    return false
end

function Input:processEvent(e, a, b, c, d)
    -- Nothing to do
end

return Input
