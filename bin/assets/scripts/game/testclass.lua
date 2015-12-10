local class = require 'nx.class'
local TestClass = class 'game.testclass'

function TestClass:initialize()
    self.testVal1 = 'hello'
    self.testVal2 = 'world'
end

function TestClass:sayIt()
    print(self.testVal1, self.testVal2)
end

return TestClass