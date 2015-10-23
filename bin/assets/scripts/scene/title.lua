local Scene = require 'nx.scene'
local Keyboard = require 'nx.keyboard'
local Mouse = require 'nx.mouse'
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

function SceneTitle:onTextInput(text)
    print('Text entered: ' .. text)
end

function SceneTitle:onTextEdit(text, start, length)
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

    if scancode == 'Backspace' then
        print('Ctrl ' .. tostring(Keyboard.modKeyDown('ctrl')) ..
            '; Alt ' .. tostring(Keyboard.modKeyDown('alt')))
    elseif scancode == 'Return' then
        print('text input: ' .. tostring(Keyboard.isTextInputActive()))
        if (Keyboard.isTextInputActive()) then
            Keyboard.stopTextInput()
        else
            Keyboard.startTextInput(10, 10, 100, 30)
        end
    elseif scancode == 'F1' then
        if Mouse.getCursor() == 'wait' then
            Mouse.setCursor('default')
        else
            Mouse.setCursor('wait')
        end
    end
end

return SceneTitle