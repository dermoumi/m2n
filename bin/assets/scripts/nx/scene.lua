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

function Scene:processWholeStack()
    return true
end

return Scene