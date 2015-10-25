local class = require "class"
local PSCLexer = require "PSCLexer"
local lexer = PSCLexer:new()
local precedence = {
	["*"] = 1;
	["/"] = 1;
	["+"] = 2;
	["-"] = 2;
}

local PSCNode = require "PSCNode"

local PSCParser = class(require "parserLib".Parser)
function PSCParser:init(lexer, defaultState)
	if not lexer then lexer = PSCLexer:new() end
	self.Super.init(self,lexer, defaultState or "main")

	self
		:defineState "main" {
			["function"] = function(parser)
				local name = parser:expect "word".match
				local arguments = {}
				parser:expect "("
				local first = true
				while not parser:accept ")" and (first or parser:expect ",") do
					local name = parser:expect "word".match
					parser:expect ":"
					local typename = parser:expect "word".match
					arguments[#arguments+1] = {
						name = name;
						vartype = typename;
					}
				end
				parser:expect ":"
				local returnType = parser:expect "word".match
				local instructions = {}
				while true do
					local instruction = parser:parse "block"
					if not instruction then parser:error "Unexpected end of file" end
					--print(instruction.type, instruction.expression.match)
          if instruction == "end" then 
						break 
					end
					instructions[#instructions+1] = instruction
					if instruction.hasEnd then
            break
					end
				end

				--print("Function: "..name.." returning "..returnType)
				return PSCNode.functionDef:new {
					name = name;
					sourceInfo = parser:getCurrentSourceInfo();
					returnType = returnType;
					instructions = instructions;
					arguments = arguments;
				}
			end;
		}
		:defineFunction("readFunctionCallArguments", function(parser, functionCallName)
			local args = {}
			repeat
				local arg = parser:parse "expression"
				args[#args + 1] = arg
				nextToken = parser:expect{",",")"}
			until nextToken.identifier == ")"
			return PSCNode.functionCall:new {
				name = functionCallName;
				sourceInfo = parser:getCurrentSourceInfo();
				arguments = args;
			}
		end)
		:defineState "block" {
			["word"] = function(parser, identifier, match)
				local nextToken = parser:next()
				if nextToken.identifier == "(" then
					return parser:parse("readFunctionCallArguments", match)
				end
				parser:error("Unexpected identifier: "..nextToken.identifier)
			end;
			["return"] = function(parser)
        if parser:accept "end" then
					return PSCNode.functionReturn:new {
						sourceInfo = parser:getCurrentSourceInfo();
						hasEnd = true;
					}
				else
					return PSCNode.functionReturn:new {
						sourceInfo = parser:getCurrentSourceInfo();
						expression = parser:parse "expression";
					}
				end
			end;
			["end"] = function(parser) 
				return "end"
			end;
		}
		:defineState "expression" {
			["literal"] = function(parser, identifier, match)
				local root = PSCNode.value:new {
					valuetype = identifier;
					sourceInfo = parser:getCurrentSourceInfo();
					value = match;
				}
				local op = parser:accept{"*","+","/"}
				if op then 
					root = PSCNode.operator:new {
						op = op.match;
						left = root;
						sourceInfo = parser:getCurrentSourceInfo();
						right = parser:parse "expression";
					}
					if root.right.op and precedence[root.op] < precedence[root.right.op] then
						local newRoot = PSCNode.operator:new {
							sourceInfo = parser:getCurrentSourceInfo();
							op = root.right.op;
							left = root;
							right = root.right.right;
						}
						root.right = root.right.left
						root = newRoot
					end
				elseif parser:accept "(" then
					return parser:parse("readFunctionCallArguments", match)
				end
				return root
			end;
			["string"] = "literal";
			["word"] = "literal";
			["integerDec"] = "literal";

		}
end

return PSCParser