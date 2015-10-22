local class = require 'nx.class'
local Scene = class 'nx.scene'

local sceneStack = {}

function Scene.static.currentScene()
    return sceneStack[#sceneStack]
end

function Scene.static.goTo(scene)
    sceneStack = {scene}
end

function Scene.static.push(scene)
    sceneStack[#sceneStack + 1] = scene
end

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

function Scene:load()
    -- Nothing to do
end

function Scene:update(dt)
    -- Nothing to do
end

function Scene:render()
    -- Nothing to do
end

function Scene:onQuit()
    -- Nothing to do
end

function Scene:onFocus(hasFocus)
    -- Nothing to do
end

function Scene:onVisible(isVisible)
    -- Nothing to do
end

function Scene:onTextInput(text)
    -- Nothing to do
end

function Scene:onTextEdit(text, start, length)
    -- Nothing to do
end

function Scene:onMouseFocus(hasFocus)
    -- Nothing to do
end

function Scene:onKeyDown(scancode, keysym, repeated)
    -- Nothing to do
end

function Scene:onKeyUp(scancode, keysym, repeated)
    -- Nothing to do
end

function Scene:onMouseMotion(x, y)
    -- Nothing to do
end

function Scene:onMouseDown(x, y, button)
    -- Nothing to do
end

function Scene:onMouseUp(x, y, button)
    -- Nothing to do
end

function Scene:onWheelScroll(x, y)
    -- Nothing to do
end

function Scene:onJoyMotion(which, axis, value)
    -- Nothing to do
end

function Scene:onJoyButtonDown(which, button)
    -- Nothing to do
end

function Scene:onJoyButtonUp(which, button)
    -- Nothing to do
end

function Scene:onJoyConnect(which, isConnected)
    -- Nothing to do
end

function Scene:onTouchDown(finger, x, y)
    -- Nothing to do
end

function Scene:onTouchUp(finger, x, y)
    -- Nothing to do
end

function Scene:onTouchMotion(finger, x, y)
    -- Nothing to do
end

function Scene:onFileDrop(file)
    -- Nothing to do
end

function Scene:onEvent(e, a, b, c, d)
    -- Nothing to do
end

function Scene:processWholeStack()
    return true
end

return Scene