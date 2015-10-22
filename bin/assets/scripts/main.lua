local Timer = require 'nx.timer'
local Window = require 'nx.window'
local Events = require 'nx.events'
local Scene = require 'nx.scene'
local SceneTitle = require 'scene.title'

Window.create("m2n", 1280, 720, false)

Scene.goTo(SceneTitle:new())

while Window.isOpen() do
    -- Process events
    for e, a, b, c in Events.poll() do
        if e == 'quit' then
            if Scene.call('onQuit') then Window.close() end
        elseif e == 'focus' then
            Scene.call('onFocus', a)
        elseif e == 'visible' then
            Scene.call('onVisible', a)
        elseif e == 'textinput' then
            Scene.call('onTextInput', a)
        elseif e == 'textedit' then
            Scene.call('onTextEdit', a, b, c)
        elseif e == 'keydown' then
            Scene.call('onKeyDown', a, b, c)
        elseif e == 'keyup' then
            Scene.call('onKeyUp', a, b, c)
        elseif e == 'mousefocus' then
            Scene.call('onMouseFocus', a)
        elseif e == 'mousemotion' then
            Scene.call('onMouseMotion', a, b)
        elseif e == 'mousedown' then
            Scene.call('onMouseDown', a, b, c)
        elseif e == 'mouseup' then
            Scene.call('onMouseUp', a, b, c)
        elseif e == 'wheelscrolled' then
            Scene.call('onWheelScroll', a, b)
        elseif e == 'joymotion' then
            Scene.call('onJoyMotion', a, b, c)
        elseif e == 'joybuttondown' then
            Scene.call('onJoyButtonDown', a, b)
        elseif e == 'joybuttonup' then
            Scene.call('onJoyButtonUp', a, b)
        elseif e == 'joyconnect' then
            Scene.call('onJoyConnect', a, b)
        elseif e == 'touchdown' then
            Scene.call('onTouchDown', a, b, c)
        elseif e == 'touchup' then
            Scene.call('onTouchUp', a, b, c)
        elseif e == 'touchmotion' then
            Scene.call('onTouchMotion', a, b, c)
        elseif e == 'filedrop' then
            Scene.call('onFileDrop', a)
        else
            Scene.call('onEvent', e, a, b, c)
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