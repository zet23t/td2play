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

local InstructionCode = require "pbs.InstructionCode"


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