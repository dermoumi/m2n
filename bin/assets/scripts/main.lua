local Timer = require 'nx.timer'
local Window = require 'nx.window'
local Events = require 'nx.events'
local Scene = require 'nx.scene'
local Nx = require 'nx'
local SceneTitle = require 'scene.title'

Window.create("m2n", 1280, 720, false)

Scene.goTo(SceneTitle:new())

local eventMapping = {
    focus             = 'onFocus',
    visible           = 'onVisible',
    textinput         = 'onTextInput',
    textedit          = 'onTextEdit',
    keydown           = 'onKeyDown',
    keyup             = 'onKeyUp',
    mousefocus        = 'onMouseFocus',
    mousemotion       = 'onMouseMotion',
    mousedown         = 'onMouseDown',
    mouseup           = 'onMouseUp',
    wheelscroll       = 'onWheelScroll',
    joyaxismotion     = 'onJoyAxisMotion',
    joyballmotion     = 'onJoyBallMotion',
    joyhatmotion      = 'onJoyHatMotion',
    joybuttondown     = 'onJoyButtonDown',
    joybuttonup       = 'onJoyButtonUp',
    joyconnect        = 'onJoyConnect',
    gamepadmotion     = 'onGamepadMotion',
    gamepadbuttondown = 'onGamepadButtonDown',
    gamepadbuttonup   = 'onGamepadButtonUp',
    gamepadconnect    = 'onGamepadConnect',
    gamepadremap      = 'onGamepadRemap',
    touchdown         = 'onTouchDown',
    touchup           = 'onTouchUp',
    touchmotion       = 'onTouchMotion',
    filedrop          = 'onFileDrop'
}

-- Handling FPS
local maxFPS = 60
local framerate = 1 / maxFPS

local totalElapsedTime, frameCount = 0, 0
local lastTime = Nx.getSystemTime()

while Window.isOpen() do
    local currentTime = Nx.getSystemTime()
    local elapsedTime = currentTime - lastTime
    lastTime = currentTime

    -- Process events
    for e, a, b, c, d in Events.poll() do
        if e == 'quit' and Scene.call('onQuit') then
            Window.close()
            break
        else
            local event = eventMapping[e] or 'onEvent'
            Scene.call(event, a, b, c, d)
        end
    end
    
    Scene.call('update', elapsedTime)
    Scene.call('render') 
    Window.display()

    -- Calculating FPS every minute
    totalElapsedTime = totalElapsedTime + elapsedTime
    frameCount       = frameCount + 1
    if totalElapsedTime > 1 then
        Window.setTitle('m2n [' .. math.floor(1 / (totalElapsedTime / frameCount) + .5) .. ']')
        totalElapsedTime = totalElapsedTime % 1
        frameCount = 0
    end

    -- Waiting
    local sysTime = Nx.getSystemTime()
    if currentTime + framerate > sysTime then
        Nx.sleep(currentTime - sysTime + framerate)
    end
end

return 0