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
-- A base class for the different scenes in the game
------------------------------------------------------------
local class = require 'nx.class'
local Scene = class 'nx.scene'

-- Local variables -----------------------------------------
local sceneStack = {}

------------------------------------------------------------
function Scene.static.currentScene()
    return sceneStack[#sceneStack]
end

------------------------------------------------------------
function Scene.static.goTo(scene)
    sceneStack = {scene}
    scene:load()
end

------------------------------------------------------------
function Scene.static.push(scene)
    sceneStack[#sceneStack + 1] = scene
    scene:load()
end

------------------------------------------------------------
function Scene.static.call(func, ...)
    local currentScene = Scene.currentScene()

    if not currentScene:processWholeStack() then
        return currentScene[func](currentScene, ...) ~= false
    else
        for i = #sceneStack, 1, -1 do
            if sceneStack[i][func](sceneStack[i], ...) == false then
                return false
            end
        end
    end

    return true
end

------------------------------------------------------------
function Scene:load()
    -- Nothing to do
end

------------------------------------------------------------
function Scene:update(dt)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:render()
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onQuit()
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onFocus(hasFocus)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onVisible(isVisible)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onResize(w, h)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTextInput(text)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTextEdit(text, start, length)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onMouseFocus(hasFocus)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onKeyDown(scancode, keysym, repeated)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onKeyUp(scancode, keysym, repeated)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onMouseMotion(x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onMouseDown(x, y, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onMouseUp(x, y, button)
    -- Nothing to do
------------------------------------------------------------
end

------------------------------------------------------------
function Scene:onWheelScroll(x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyAxisMotion(which, axis, value)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyBallMotion(which, ball, xrel, yrel)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyHatMotion(which, hat, value)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyButtonDown(which, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyButtonUp(which, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onJoyConnect(which, isConnected)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadMotion(which, axis, value)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadButtonDown(which, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadButtonUp(which, button)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadConnect(which, isConnected)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onGamepadRemap(which)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTouchDown(finger, x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTouchUp(finger, x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onTouchMotion(finger, x, y)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onFileDrop(file)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:onEvent(e, a, b, c, d)
    -- Nothing to do
end

------------------------------------------------------------
function Scene:processWholeStack()
    return true
end

------------------------------------------------------------
return Scene