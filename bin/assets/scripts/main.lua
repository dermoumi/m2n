class = require 'middleclass'

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

return 0