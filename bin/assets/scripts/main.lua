local Log = require 'nx.log'
Log.error("Hello world " .. 103)

local Fs = require 'nx.filesystem'
local assets = Fs.enumerateFiles('assets');
for i,v in ipairs(assets) do
    Log.info("Asset file: " .. v);
end

Log.info("Is assets/scripts a file? " .. tostring(Fs.isFile('assets/scripts')))
Log.info("Is assets/scripts a folder? " .. tostring(Fs.isDirectory('assets/scripts')))
Log.info("Is main.lua a file? " .. tostring(Fs.isFile('assets/scripts/main.lua')))
Log.info("Is main.lua a folder? " .. tostring(Fs.isDirectory('assets/scripts/main.lua')))

local OutputFile = require 'nx.outputfile'
local file = OutputFile:new()
local ok, err = file:open('test.txt')
if not ok then
    Log.error('Could not open file: ' .. err)
else
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
local file = InputFile:new()
ok, err = file:open('userdata/test.txt')
if not ok then
    Log.error('Could not open file: ' .. err)
    return 0
end

local LuaVM = require 'nx.luavm'
local vm = LuaVM:new()
if not vm:isOpen() then
    error('erm?')
end

local retCount, err = vm:call(function(file)
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
    
    return ':)'
end, file)
file:close()

if not retCount then print(err) end

print(vm:pop(retCount, true))

local Nx = require 'nx'
Log.info('Current time: ' .. tostring(Nx.getSystemTime()))
local time = Nx.getSystemTime()
Nx.sleep(0.7)
Log.info('Time passed: ' .. tostring(Nx.getSystemTime() - time))
Log.info('Current time: ' .. tostring(Nx.getSystemTime()))

return 0