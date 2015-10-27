local class = require "class"

local PBSType = class()
function PBSType:init(name, sizeBytes)
	self.name = name
	self.sizeBytes = sizeBytes
end

local builtinTypes = {
	PBSType:new("void", 0)
}

for primitive in ("int uint"):gmatch "%S+" do
	for size in ("8 16 32"):gmatch "%S+" do
		builtinTypes[#builtinTypes + 1] = PBSType:new(primitive..size, size / 8)
	end
end
builtinTypes[#builtinTypes + 1] = PBSType:new("float",32)
builtinTypes[#builtinTypes + 1] = PBSType:new("ufloat",32)


local InstructionCode = class()

InstructionCode.opCodeMap = {
	["push-8bit-literal"] = 0x01;
	["push-16bit-literal"] = 0x02;
	["push-32bit-literal"] = 0x03;
	["push-string-literal"] = 0x04;
	["push-8bit-value"] = 0x05;
	["push-16bit-value"] = 0x06;
	["push-stack-value-uint8"] = 0x07;
	["op-*-uint8-uint8-uint16"] = 0x08;
	["return-uint16"] = 0x09;
	["call"] = 0x10;
	["callNative"] = 0x11;
	["return-void"] = 0x12;
  ["move-function-stack-offset"] = 0x13;
  ["cast-uint16-int16"] = 0x14;
  ["cast-uint16-int32"] = 0x15;
}
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


local PBSCompiler = class()

function PBSCompiler:init(parser)
	self.parser = parser or require "pbs.PBSParser":new()
	self.typeMap = {}
	for i,type in ipairs(builtinTypes) do
		self:addType(type)
	end
	self.functionMap = {}
	self.nativeFunctionMap = {}
end

function PBSCompiler:addNativeFunction(address, name, returnType)
	local funcDef = {
		address = address; 
		name = name; 
		arguments = {};
		returnType = returnType;
		isNative = true; 
	}
	assert(not self.nativeFunctionMap[name], "Native function already defined: "..name)
	self.nativeFunctionMap[name] = funcDef
	assert(self.typeMap[returnType], "Unknown returnType: "..returnType)
	local function addArg(vartype)
		local n = #funcDef.arguments+1
		assert(self.typeMap[returnType], "Unknown argument type for arg "..n..": "..vartype)
		funcDef.arguments[n] = {
			name = "#"..n;
			vartype = vartype;
		}
		return addArg
	end
	return addArg
end

function PBSCompiler:sizeOf(type)
	return self.typeMap[type].sizeBytes
end

function PBSCompiler:addType(type)
	self.typeMap[type.name] = type
	return self
end

function PBSCompiler:compile(file)
	self.parser:load(file)
	local ast = {}
	repeat
		local result = self.parser:parse "main"
		if result then
			ast[#ast+1] = result
		end
	until not result
	return self:process(ast)
end

function PBSCompiler:processFunctionDeclaration(ast)
	for i,node in ipairs(ast) do
		if node.type == "functionDef" then
			local fdecl = self.functionMap[node.name]
			if fdecl then
				error(
					"Function name '"..node.name.."' can't be declared ("..node.sourceInfo
						.."). First use: "..fdecl.sourceInfo)
			end
			self.functionMap[node.name] = node
		end
	end
end

function PBSCompiler:processFunctionImplementation(funcNode, instructionCode)
	local variableMapStack = {{}}
	instructionCode:flagFunctionStart(funcNode.name)
  
	funcNode.instructionCode = instructionCode
	local stackpos = 0
	local function getVariable(name)
		for i=#variableMapStack,1,-1 do
			if variableMapStack[i][name] then
				return variableMapStack[i][name]
			end
		end
	end
	local function addVariable(name,vartype)
		variableMapStack[#variableMapStack][name] = {
			name = name;
			vartype = vartype;
			stackpos = stackpos;
		}
		stackpos = stackpos + self:sizeOf(vartype)
	end

	-- add arguments to variable stack
	for i,arg in ipairs(funcNode.arguments) do
		addVariable(arg.name, arg.vartype)
	end
  
  if stackpos ~= 0 then 
    instructionCode:pushMoveFunctionOffset(stackpos)
  end

	-- process instructions
	local processor = {}
	local function process(node,...) 
		return assert(processor[node.type],"No processor for "..node.type)(node,...)
	end
	function processor.operator(node, targetType)
		local lefttype = process(node.left, targetType)
		local righttype = process(node.right, targetType)
		instructionCode:pushOperator(node.op, lefttype, righttype, targetType)
		if leftType == righttype then return leftType end
		return "?"
	end
	function processor.functionCall(node, targetType)
    assert(targetType)
		local target = self.functionMap[node.name] or self.nativeFunctionMap[node.name]
		if not target then
			error("Unknown function to call: "..node.name.." "..node.sourceInfo)
		end
		if #target.arguments ~= #node.arguments then 
			error("target function expects "..#target.arguments.." argument(s), "..#node.arguments.." given "..node.sourceInfo)
		end
		for i,arg in ipairs(node.arguments) do
			local targetArg = target.arguments[i]
      local t = process(arg,targetArg.vartype)
		end
		if target.isNative then
			instructionCode:pushNativeFunctionCall(target.address)
		else
			instructionCode:pushFunctionCall(node.name, #node.arguments)
		end
    if targetType ~= target.returnType then
      instructionCode:pushCast(target.returnType, targetType)
    end
    --return target.returnType
	end
	function processor.functionReturn(node)
		if node.expression then
			local type = process(node.expression, funcNode.returnType)
			instructionCode:returnFromCall(funcNode.returnType)
		else
			instructionCode:returnFromCall "void"
		end
	end
	function processor.value(node, targetType)
    assert(targetType)
		if node.isLiteral then
			--print(node.value,node.valuetype)
			if node.valuetype == "integerDec" then
				local allowed = {
					uint8 = true, uint16 = true, uint32 = true,
					int8 = true, int16 = true, int32 = true
				}
				assert(allowed[targetType], "Target type incompatible with integer value: "..targetType)
				if targetType == "uint8" or targettype == "int8" then
					instructionCode:push8bitLiteral(node.value)
				elseif targettype == "uint16" or targettype == "int16" then
					instructionCode:push16bitLiteral(node.value)
				else
					instructionCode:push32bitLiteral(node.value)
				end
				return targettype
			elseif node.valuetype == "string" then
				instructionCode:pushStringLiteral(node.value)
				return "string"
			else
				error("Unsupported value type: "..node.valuetype)
			end
		else
			local var = getVariable(node.value)
			if not var then 
				error("Variable without definition: "..node.value.." "..node.sourceInfo) 
			end
			node.var = var
			instructionCode:pushStackValue(var.stackpos, var.vartype)
			return var.vartype
			--print(getVariable(node.value).vartype)
		end
	end
	for i=1,#funcNode.instructions do
		local inst = funcNode.instructions[i]
		--print(node.type,node.sourceInfo)
		process(inst, "void")
	end
end

function PBSCompiler:process(ast)
	local instructionCode = InstructionCode:new()
	self.instructionCode = instructionCode
	self:processFunctionDeclaration(ast)
	assert(self.functionMap.main, "No main function defined!")
	-- put main function declaration to address 0
	self:processFunctionImplementation(self.functionMap.main, instructionCode)
	for name, node in pairs(self.functionMap) do
		if name ~= "main" then
			self:processFunctionImplementation(node, instructionCode)
		end
	end
	--instructionCode:printInstructions()
	return instructionCode:compileCode()
end

return PBSCompiler