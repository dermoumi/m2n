local Timer = require 'nx.timer'
local Window = require 'nx.window'
local Events = require 'nx.events'
local Scene = require 'nx.scene'
local SceneTitle = require 'scene.title'

local Thread = require 'nx.thread'
print('main: is main? ' .. tostring(Thread.isMain()))
local thread = Thread:new(function()
    local Thread = require 'nx.thread'
    print('thread: is main? ' .. tostring(Thread.isMain()))
end)
thread:join()

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
end

--[[
local file1, err = OutputFile:new('test.txt')
if not file1 then
    Log.error('Could not open file: ' .. err)
    return 1
end
    file:write("hello world?", 12)

    file:writeS8(1)
    file:writeS16(2)
    file:writeS32(3)
    file:writeU8(4)
    file:writeU16(5)
    file:writeU32(6)
    file:writeFloat(7.89)
    file:writeDouble(10.1112)
    file:writeString('Héllô Dáñknèss 日本語 !?')
    
    file:close()
end

local InputFile = require 'nx.inputfile'
file, err = InputFile:new('userdata/test.txt')
if not file then
    Log.error('Could not open file: ' .. err)
    return 1
end

local LuaVM = require 'nx.luavm'
local vm, err = LuaVM:new()
if not vm then
    error('erm? ' .. err)
end

local testvm, err = LuaVM:new()
if not testvm then
    error('erm? ' .. err)
end

local retCount, err = vm:call(function(file, vm)
    local Log = require 'nx.log'

    Log.info(file:read(12))

    Log.info(file:readS8())
    Log.info(file:readS16())
    Log.info(file:readS32())
    Log.info(file:readU8())
    Log.info(file:readU16())
    Log.info(file:readU32())
    Log.info(file:readFloat())
    Log.info(file:readDouble())
    Log.info(file:readString())
    
    vm:call(function() print('hello everyone :D') end)

    return ':)'
end, file, testvm)

testvm = nil
file:close()

if not retCount then print(err) end

print(vm:pop(retCount, true))

local Nx = require 'nx'
Log.info('Current time: ' .. tostring(Nx.getSystemTime()))
local time = Nx.getSystemTime()
Nx.sleep(0.7)
Log.info('Time passed: ' .. tostring(Nx.getSystemTime() - time))
Log.info('Current time: ' .. tostring(Nx.getSystemTime()))

]]--

return 0