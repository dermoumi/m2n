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

firstInput = nil

function Input.static.register(input)
    if firstInput then
        firstInput.next = input
        input.prev = firstInput
    else
        firstInput = input
    end
end

function Input.static.unregister(input)
    if firstInput == input then
        firstInput = input.next
        firstInput.prev = nil
    else
        input.prev.next = input.next
        if input.next then
            input.next.prev = input.prev
        end
    end
end

function Input.static.processEvent(e, a, b, c, d)
    if firstInput then
        firstInput:process(function(e, a, b, c, d)
            require('screen').currentScreen():__onEvent(e, a, b, c, d)
        end, e, a, b, c, d)
    end
end

function Input:process(cb, e, a, b, c, d)
    if self.next then self.next:processEvent(cb, e, a, b, c, d) end

    -- Nothing else to do?
end

function Input:down(button)
    return false
end

function Input:position(stick)
    return 0, 0
end

return Input
