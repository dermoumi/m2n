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

local Cache = {}

local Log = require 'util.log'

local items = {}

function Cache.get(id, loadFunc, addCount)
    local item = items[id]

    -- If item does not exist, try to load it using loadFunc
    if not item then
        -- If no load func, abandon
        if not loadFunc then
            return nil, 'Invalid loading function'
        end

        -- Try to load the object
        local newObj, err = loadFunc()
        if not newObj then
            return nil, err or 'An error occurred while loading "' .. id .. '"'
        end

        -- Add the new object to the cache
        item = Cache.add(id, newObj)
    end

    -- If requested, increment the load count of the item
    if addCount then
        item.loadCount = item.loadCount + 1
    end

    -- Return the item's object
    return item.obj
end

function Cache.release(id)
    local item = items[id]
    if not item then return end

    -- Decrement load count
    item.loadCount = item.loadCount - 1

    -- If load count reaches zero, remove item from list
    if item.loadCount <= 0 then
        Log.info('Removing from cache: ' .. id)

        items[id] = nil

        -- If object can be released, do that
        if item.obj.release then item.obj:release() end
    end
end

function Cache.add(id, newObj)
    Log.info('Adding to cache: ' .. id)

    local item = {
        loadCount = 0,
        obj = newObj
    }

    items[id] = item
    return item
end

return Cache
