local Timer = require 'nx.timer'
local Window = require 'nx.window'
local Events = require 'nx.events'
local Scene = require 'nx.scene'
local SceneTitle = require 'scene.title'

local InputFile = require 'nx.inputfile'
local file, err = InputFile:new('/sysroot/home/sid/test')
if not file then
    print('Cannot open file: ' .. err)
else
    print('File content: ' .. file:read())
end

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

while Window.isOpen() do
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
    
    Scene.call('update', 0)
    Scene.call('render') 
    Window.display()
end

return 0