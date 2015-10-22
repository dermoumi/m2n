local Scene = require 'nx.scene'
local class = require 'nx.class'

local SceneTitle = class('scene.title', Scene)

function SceneTitle:onQuit()
    print('quitting?')
end

function SceneTitle:onFocus(hasFocus)
    print('Window has focus? ' .. tostring(hasFocus))
end

function SceneTitle:onVisible(isVisible)
    print('Window is visible? ' .. tostring(isVisible))
end

function SceneTitle:onTextEntered(text)
    print('Text entered: ' .. text)
end

function SceneTitle:onTextEdited(text, start, length)
    print('Text edited from ' .. start .. ' to ' .. (start + length) .. ': ' .. text)
end

function SceneTitle:onMouseFocus(hasFocus)
    print('Has mouse focus? ' .. tostring(hasFocus))
end

function SceneTitle:onMouseDown(x, y, button)
    print('Mouse button pressed at: ' .. x .. ', ' .. y .. ' Button: ' .. button)
end

function SceneTitle:onMouseUp(x, y, button)
    print('Mouse button released at: ' .. x .. ', ' .. y .. ' Button: ' .. button)
end

function SceneTitle:onTouchDown(finger, x, y)
    print('Touch pressed at: ' .. x .. ', ' .. y .. ', finger: ' .. finger)
end

function SceneTitle:onTouchUp(finger, x, y)
    print('Touch released at: ' .. x .. ', ' .. y .. ', finger: ' .. finger)
end

function SceneTitle:onKeyDown(scancode, keysym, repeated)
    print('Key pressed: ' .. scancode .. ' sym: ' .. keysym .. ' isRepeat? ' .. tostring(repeated))
end

return SceneTitle