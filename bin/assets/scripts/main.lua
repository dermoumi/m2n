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

local Nx     = require 'nx'
local Window = require 'nx.window'
local Events = require 'nx.events'
local Scene      = require 'scene'
local SceneTitle = require 'scene.title'

------------------------------------------------------------
-- Helpers
------------------------------------------------------------

-- Mapping events to their respecting nx.Scene functions
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

-- Helper function to determine what the default FPS should be
function getMaxFPS()
    local platform = Nx.getPlatform()
    if platform == 'android' or platform == 'ios' then
        return 30
    else
        return 60
    end
end

------------------------------------------------------------
-- Window creation
------------------------------------------------------------
Window.create("m2n", 1280, 720, false)

------------------------------------------------------------
-- Startup scene
------------------------------------------------------------
Scene.goTo(SceneTitle:new())

------------------------------------------------------------
-- Handling FPS
------------------------------------------------------------
local maxFPS = getMaxFPS()
local framerate = 1 / maxFPS

local totalElapsedTime, frameCount = 0, 0
local lastTime = Nx.getSystemTime()

------------------------------------------------------------
-- Main loop
------------------------------------------------------------
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
        local currentFPS = math.floor(1 / (totalElapsedTime / frameCount) + .5)
        Window.setTitle('m2n [' .. currentFPS .. ']')

        -- Print the FPS when on android
        if Nx.getPlatform() == 'android' then
            print('Current FPS: ' .. currentFPS)
        end
        
        totalElapsedTime = totalElapsedTime % 1
        frameCount = 0
    end

    -- Waiting
    local sysTime = Nx.getSystemTime()
    if currentTime + framerate > sysTime then
        Nx.sleep(currentTime - sysTime + framerate)
    end
end

------------------------------------------------------------
-- The end?
------------------------------------------------------------
return 0