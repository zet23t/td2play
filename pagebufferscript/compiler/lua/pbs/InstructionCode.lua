
local InstructionCode = class()

InstructionCode.opCodeMap = require "pbs.const.opCodeMap"
function InstructionCode:init()
	self.instructionList = {}
	self.code = {}
	-- a mapping of strings to memory addresses
	self.stringMemoryMap = {}
end
function InstructionCode:addInstruction(instruction)
	self.instructionList[#self.instructionList + 1] = instruction
end
function InstructionCode:flagFunctionStart(name)
	self:addInstruction {"functionstart",name; size = 0}
end
function InstructionCode:push8bitLiteral(byte)
	self:addInstruction {"push-8bit-literal", tonumber(byte); size = 1 + 1}
end
function InstructionCode:push16bitLiteral(byte2)
	self:addInstruction {"push-16bit-literal", tonumber(byte2); size = 1 + 2}
end
function InstructionCode:push32bitLiteral(byte4)
	self:addInstruction {"push-32bit-literal", tonumber(byte4); size = 1 + 4}
end
function InstructionCode:pushStringLiteral(string)
	self.stringMemoryMap[string] = 0
	self:addInstruction {"push-string-literal", string; size = 1 + 2}
end
function InstructionCode:pushMoveFunctionOffset(offset)
  self:addInstruction {"move-function-stack-offset", offset; size = 1 + 3}
end
function InstructionCode:pushNativeFunctionCall(address)
	self:addInstruction {"callNative", address; size = 1 + 1}
end
function InstructionCode:pushCast(from,to)
  self:addInstruction {"cast-"..from.."-"..to, size = 1}
end
function InstructionCode:pushFunctionCall(name, argCount)
	self:addInstruction {"call", name, argCount; size = 1 + 2}
end
function InstructionCode:pushOperator(op, leftType, rightType, targetType)
	self:addInstruction {"op-"..op.."-"..leftType.."-"..rightType.."-"..targetType; size = 1}
end
function InstructionCode:pushStackValue(stackpos, vartype)
	self:addInstruction {"push-stack-value-"..vartype, stackpos; size = 2}
end
function InstructionCode:returnFromCall(type)
	self:addInstruction {"return-"..type; size = 1}
end
function InstructionCode:printInstructions()
	for i=1,#self.instructionList do
		print(unpack(self.instructionList[i]))
	end
end
function InstructionCode:compileCode()
	self.code = {}
	local function put8(byte)
    if type(byte) == "number" then byte = string.char(byte) end
		self.code[#self.code+1] = byte
	end
	local function put16(byte2)
		if type(byte2) == "number" then
			byte2 = string.char(byte2 % 0x100) .. string.char(byte2 / 0x100)
		end
		put8(byte2:sub(1,1))
		put8(byte2:sub(2,2))
	end
	local function put32(byte4)
		if type(byte4) == "number" then
			byte4 = string.char(byte4 % 0x100) 
				..  string.char(math.floor(byte4 / 0x100) % 0x100)
				..  string.char(math.floor(byte4 / 0x10000) % 0x100)
				..  string.char(math.floor(byte4 / 0x1000000) % 0x100)
		end
		put8(byte4:sub(1,1))
		put8(byte4:sub(2,2))
		put8(byte4:sub(3,3))
		put8(byte4:sub(4,4))
	end
	local pos = 0
	local functionMemoryAddressMap = {}
  self.functionAddressMarkerMap = {}
	for i=1,#self.instructionList do
		local inst = self.instructionList[i]
		if inst[1] == "functionstart" then 
			functionMemoryAddressMap[inst[2]] = pos
      self.functionAddressMarkerMap[pos] = inst[2]
		end
    --print("position: ",pos,inst[1],inst.size)
		pos = pos + inst.size
	end
	for i=1,#self.instructionList do
		local inst = self.instructionList[i]
		local op = inst[1]
		if op ~= "functionstart" and inst.size > 0 then
			put8(assert(self.opCodeMap[op], "no op: "..op))
			if     op == "push-8bit-value" 
				or op == "push-16bit-value"  
				or op == "push-8bit-literal" 
				or op == "push-stack-value-uint8" 
			then
        --print("?", op, inst[2])
				put8(inst[2])
			elseif op == "push-16bit-literal" then
				put16(inst[2])
			elseif op == "push-32bit-literal" then
				put32(inst[2])
			elseif op == "push-string-literal" then
				put16(self.stringMemoryMap[inst[2]])
			elseif op == "call" then
				put16(assert(functionMemoryAddressMap[inst[2]]))
			elseif op == "callNative" then
				put8(inst[2])
			elseif op == "op-*-uint8-uint8-uint16" 
				or op == "return-uint16" 
        or op == "return-void" then
				-- no param, just op
      elseif op == "move-function-stack-offset" then
        put16(inst[2])
      elseif op == "cast-uint16-int16" then
        -- nop
      elseif op == "cast-uint16-int32" then
			else
				error("no handling for "..op)
			end
		end
	end
	return table.concat(self.code)
end

return InstructionCode