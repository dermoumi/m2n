local Scene = require 'nx.scene'
local class = require 'nx.class'

local SceneTitle = class('scene.title', Scene)

function SceneTitle:onQuit()
    print('quitting?')
    return true
end

return SceneTitle