local PSCCompiler = require "PSCCompiler"
local prog = PSCCompiler:new()
prog:addNativeFunction(0, "printInt16","void") "int16"
prog:addNativeFunction(1, "printInt32","void") "int32"
local code = prog:compile "test/funcdef.psc"
prog.instructionCode:printInstructions()
io.write("// Program size: "..#code.." bytes")
for i=1,#code do
  local fname = prog.instructionCode.functionAddressMarkerMap[i-1]
  if fname then
    io.write("\n// function: "..fname.."@"..(i-1).."\n")
  end
	io.write(("0x%02x, "):format(code:byte(i)))
end
print()


--[[
local PSCParser = require "PSCParser"
local parser = PSCParser:new()

parser:load "test/funcdef.psc"
local program = {}
repeat
	local result = parser:parse "main"
	if result then
		program[#program+1] = result
	end
until not result

local function prettyPrint(node, indent)
	indent = (indent or "") .. "  "
	if node.type == "functionDef" then
		print(indent.."Function: "..node.name.." "..node.sourceInfo)
		print(indent.." Arguments: ")
		for i=1,#node.arguments do
			print(indent.."  "..node.arguments[i].name)
		end
		print(indent.." Returning: "..node.returnType)
		for i=1,#node.instructions do 
			prettyPrint(node.instructions[i],indent)
		end
	end
	if node.type == "functionCall" then
		print(indent.."Call: "..node.name)
		print(indent.." Arguments: ")
		for i=1,#node.arguments do 
			prettyPrint(node.arguments[i],indent)
		end
	end
	if node.type == "value" then
		print(indent.."Value: "..node.value..": "..node.valuetype.." "..node.sourceInfo)
	end
	if node.type == "operator" then
		print(indent.."Operator: "..node.op)
		prettyPrint(node.left,indent)
		prettyPrint(node.right,indent)
	end
end

for i,node in ipairs(program) do 
	prettyPrint(node)
end

local reduced = {}
for i,node in ipairs(program) do 
	reduced[i] = node:reduce()
end
print "=== REDUCED ==="
for i,node in ipairs(reduced) do 
	prettyPrint(node)
end
]]