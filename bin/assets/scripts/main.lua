local OutputFile = require 'nx.outputfile'
local InputFile = require 'nx.inputfile'
local Thread = require 'nx.thread'
local Log = require 'nx.log'
local Mutex = require 'nx.mutex'
local ffi = require 'ffi'
local Timer = require 'nx.timer'
local Window = require 'nx.window'

Window.create("m2n", 1280, 720, false)

local mutex = Mutex:new()
local sharedVal = ffi.new('float[1]', {0})
local timer = Timer:new()

local thread1, err = Thread:new(function(sharedVal, mutex)
    local Nx = require 'nx'
    local Log = require 'nx.log'
    local ffi = require 'ffi'

    sharedVal = ffi.cast('float*', sharedVal)

    for i=1, 20 do
        Log.info('Thread 1 : ' .. i)
        Nx.sleep(0.1)
        mutex:lock()
        sharedVal[0] = sharedVal[0] + 1
        mutex:unlock()
    end

    return ':D'
end, sharedVal, mutex)

if err then
    Log.error(err)
    return 1
end

local thread2, err = Thread:new(function(id, sharedVal, mutex)
    local Nx = require 'nx'
    local Log = require 'nx.log'

    local ffi = require 'ffi'

    sharedVal = ffi.cast('float*', sharedVal)

    for i=1, 10 do
        Log.info('Thread ' .. id .. ' : ' .. i)
        Nx.sleep(0.2)
        mutex:lock()
        sharedVal[0] = sharedVal[0] + 1
        mutex:unlock()
    end

    return 'D:'
end, 'a', sharedVal, mutex)

if err then
    Log.error(err)
    return 1
end

local result1, err1 = thread1:join()
if err1 then
    Log.info('Error 1 ' .. err1)
    return 1
end

local result2, err2 = thread2:join()
if err2 then
    Log.info('Error 2 ' .. err2)
    return 1
end

Log.info('Results: ' .. result1 .. ', ' .. result2 .. ', ' .. sharedVal[0] ..
    ', Elapsed time: ' .. timer:elapsedTime())

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