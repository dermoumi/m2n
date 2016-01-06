--[[
    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or
    distribute this software, either in source code form or as a compiled
    binary, for any purpose, commercial or non-commercial, and by any
    means.

    In jurisdictions that recognize copyright laws, the author or authors
    of this software dedicate any and all copyright interest in the
    software to the public domain. We make this dedication for the benefit
    of the public at large and to the detriment of our heirs and
    successors. We intend this dedication to be an overt act of
    relinquishment in perpetuity of all present and future rights to this
    software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
    OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

    For more information, please refer to <http://unlicense.org>
--]]


local Log = require 'util.log'
local Worker = require 'game.worker'

local Cache = {}
local items = {}

function Cache.get(screen, id, peek)
    if type(screen) == 'string' then
        screen, id, peek = nil, screen, id
    end

    local item = items[id]

    -- If item does not exist, try to load it using loadFunc
    if not item then
        items[id] = item
        local obj, reusable = Worker.addLoadingTask(screen, id)
        if reusable then
            Log.info('Adding to global cache: ' .. id)
            item = {
                count = 0,
                obj = obj
            }

            items[id] = item
        else
            return obj, false
        end
    end

    -- If requested, increment the load count of the item
    if not peek then
        item.count = item.count + 1
    end

    -- Return the item's object
    return item.obj, true
end

function Cache.release(id)
    local item = items[id]
    if not item then return end

    -- Decrement load count
    item.count = item.count - 1

    -- If load count reaches zero, remove item from list
    if item.count <= 0 then
        Log.info('Removing from global cache: ' .. id)

        items[id] = nil

        -- If object can be released, do that
        if item.obj.release then item.obj:release() end
    end
end

return Cache
