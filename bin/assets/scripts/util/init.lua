local Util = {}

local function pot(val)
    local pot = 1
    while pot < val do pot = pot * 2 end
    return pot
end

function Util.pot(a, b, ...)
    if b then
        local vals = {a, b, ...}
        for i, v in ipairs(vals) do
            vals[i] = pot(v)
        end
        return unpack(vals)
    else
        return pot(a)
    end
end

return Util