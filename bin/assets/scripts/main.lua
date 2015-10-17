local OutputFile = require 'nx.outputfile'
local InputFile = require 'nx.inputfile'
local Thread = require 'nx.thread'
local Log = require 'nx.log'
local Mutex = require 'nx.mutex'
local ffi = require 'ffi'
local Timer = require 'nx.timer'
local Window = require 'nx.window'
local Events = require 'nx.events'

Window.create("m2n", 1280, 720, false)

local done = false
while not done do
    for e, a, b, c in Events.poll() do
        if e == 'quit' then
            done = true
        elseif e == 'focus' then
            print('Window has focus? ' .. tostring(a))
        elseif e == 'visible' then
            print('Window is visible? ' .. tostring(a))
        elseif e == 'textentered' then
            print('Text entered: ' .. a)
        elseif e == 'textedited' then
            print('Text edited from ' .. b .. ' to ' .. (b + c) .. ': ' .. a)
        elseif e == 'mousefocus' then
            print('Has mouse focus? ' .. tostring(a))
        elseif e == 'mousepressed' then
            print('Mouse button pressed at: ' .. a .. ', ' .. b .. ' Button: ' .. c)
        elseif e == 'mousereleased' then
            print('Mouse button released at: ' .. a .. ', ' .. b .. ' Button: ' .. c)
        elseif e == 'touchbegan' then
            print('Touch event: ' .. a .. ', ' .. b .. ', ' .. c)
        elseif e ~= 'mousemoved' then
            print('Event: ' .. e)
        end
    end
end

Window.close()

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