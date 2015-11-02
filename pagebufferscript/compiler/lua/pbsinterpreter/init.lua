local class = require "class"

local VM = class()
VM.log = require "log"

local opCodeMap = require "pbs.const.opCodeMap"
local opIds = {}
for k,v in pairs(opCodeMap) do 
  assert(not opIds[v])
  opIds[v] = k 
end

local instruction = {
  ["push-8bit-literal"] = function(vm) 
      vm:pushOnStack(vm:nextInstruction(1))
      return true
    end;
	["push-16bit-literal"] = function(vm) end;
	["push-32bit-literal"] = function(vm) end;
	["push-string-literal"] = function(vm) end;
	["push-8bit-value"] = function(vm) end;
	["push-16bit-value"] = function(vm) end;
	["push-stack-value-uint8"] = function(vm) 
      local offset = vm:nextInstruction(1)
      vm:pushStackValue(offset,1)
      return true
    end;
  ["op-*-uint8-uint8-uint16"] = function(vm) 
      local frame = vm:currentStackFrame()
      local a = vm.stack[frame.stackOffset - 1]
      local b = vm.stack[frame.stackOffset - 2]
      frame.stackOffset = frame.stackOffset - 2
      local result = a * b
      local low = result % 0x100
      local high = (result - low) / 0x100
      vm:pushOnStack(low, high)
      return true
    end;
	["return-uint16"] = function(vm) 
      vm:popStackFrame(2)
      return true
    end;
	["call"] = function(vm) 
      local address = vm:nextInstructionUInt16()
      vm:pushNewStackFrame(address)
      return true
    end;
	["callNative"] = function(vm) 
      local id = vm:nextInstruction(1)
      local fnInfo = assert(vm.nativeFunctions[id])
      vm:pushNewStackFrame()
      local frame = vm:currentStackFrame()
      frame.functionStackOffset = frame.functionStackOffset - fnInfo.argumentSize
      fnInfo.func(vm)
      vm:popStackFrame(fnInfo.returnSize)
      return true
    end;
	["return-void"] = function(vm) end;
  ["move-function-stack-offset"] = function(vm) 
      local frame = vm:currentStackFrame()
      frame.functionStackOffset = frame.functionStackOffset - vm:nextInstructionUInt16()
      return true
    end;
  ["cast-uint16-int16"] = function(vm) return true end;
  ["cast-uint16-int32"] = function(vm) 
       vm:pushOnStack(0,0)
       return true
    end;
  ["cast-int8-int16"] = function(vm) 
      vm:pushOnStack(0)
      return true
    end;
}
instruction["push-stack-value-int8"] = instruction["push-stack-value-uint8"];

function VM:init()
  self.stack = {}
  self.stackframes = {}
  self.nativeFunctions = {}
end

function VM:load(code)
  self.code = code
  self:init()
  self:pushNewStackFrame()
end

function VM:pushOnStack(...)
  local frame = self:currentStackFrame()
  for i=1,select('#',...) do
    self.stack[frame.stackOffset] = select(i,...)
    frame.stackOffset = frame.stackOffset + 1
  end
end

function VM:readInt16FromStack(offset)
  local frame = self:currentStackFrame()
  local a,b = self.stack[frame.functionStackOffset + offset], self.stack[frame.functionStackOffset + offset + 1]
  return a + b * 0x100
end

function VM:readInt32FromStack(offset)
  local frame = self:currentStackFrame()
  local i = frame.functionStackOffset + offset
  local s = self.stack
  local a,b,c,d = s[i], s[i + 1],s[i+2],s[i+3]
  return a + b * 0x100 + c * 0x10000 + d * 0x1000000
end


function VM:pushStackValue(offset, size)
  local frame = self:currentStackFrame()
  for i=1,size do
    local value = self.stack[frame.functionStackOffset - 1 + i + offset]
    self.stack[frame.stackOffset] = value
    frame.stackOffset = frame.stackOffset + 1
  end
end

function VM:pushNewStackFrame(instructionPointer)
  local current = self.stackframes[#self.stackframes]
  self.stackframes[#self.stackframes + 1] = {
    instructionPointer = instructionPointer or 0;
    stackOffset = current and current.stackOffset or #self.stack + 1; 
    functionStackOffset = current and current.stackOffset or #self.stack + 1;
  }
end

function VM:popStackFrame(keep)
  local frame = self.stackframes[#self.stackframes]
  self.stackframes[#self.stackframes]= nil
  local now = self.stackframes[#self.stackframes]
  if now then
    now.stackOffset = frame.functionStackOffset + (keep or 0)
    local toStart = now.stackOffset - keep
    local fromStart = frame.stackOffset - keep
    for i=0,keep-1 do 
      self.stack[toStart + i]= self.stack[fromStart + i]
    end
    for i=now.stackOffset,#self.stack do
      self.stack[i] = nil
    end
  end
  return frame
end
function VM:currentStackFrame()
  return self.stackframes[#self.stackframes]
end

function VM:nextInstruction(size)
  local frame = self:currentStackFrame()
  local ip = frame.instructionPointer
  frame.instructionPointer = ip + size
  return self.code:byte(ip + 1, ip + size)
end

function VM:nextInstructionUInt16()
  local a,b = self:nextInstruction(2)
  return a + b * 0x100
end

function VM:execute()
  while self:currentStackFrame() do 
    local op = assert(self:nextInstruction(1), self:currentStackFrame().instructionPointer)
    local opName = assert(opIds[op], op)
    local opFunc = assert(instruction[opName], opName)
    
    self.log:info(opName)
    self.log:info("  >",unpack(self.stack,1,self:currentStackFrame().stackOffset))
    if not opFunc(self) then
      if not opName:match "^return" then 
        self.log:warn("Terminated with "..opName)
      end
      break
    end
    self.log:info("  <",unpack(self.stack,1,self:currentStackFrame().stackOffset))
  end
end

function VM:addFunction(id, func, argumentSize, returnSize)
  self.nativeFunctions[id] = {
    func = func;
    argumentSize = argumentSize;
    returnSize = returnSize;
  }
end

return VM