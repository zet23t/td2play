local class = require "class"

local vm = class()

function vm:init()
  self.stack = {}
  self.stackframes = {}
  self.nativeFunctions = {}
end

function vm:pushNewStackFrame(instructionPointer)
  self.stackframes[#self.stackframes + 1] = {
    instructionPointer = instructionPointer;
    stackOffset = #self.stack + 1; 
    functionStackOffset = #self.stack + 1;
  }
end

function vm:exec(code)
  self.code = code
  
end

function vm:addFunction(id, func)
  self.nativeFunctions[id] = func
end