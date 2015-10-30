local parserLib = require "parserLib"
local Lexer = parserLib.Lexer
local class = require "class"

local PBSLexer = class(Lexer)

function PBSLexer:init(scanner) 
	self.Super.init(self,scanner)
	self
		:addKeywords "function" "end" "(" ")" ":" "+" "*" "/" "," "return" "local" "="
	self
		:addRecognizer("comment", "^%-%-[^\n]+.?")
		:addRecognizer("word", "^[a-zA-Z_][a-zA-Z0-9_]*")
		:addRecognizer("whitespace","^%s+")
		:addRecognizer("string", '^""')
		:addRecognizer("string", '^"[^\n]-[^\\]"')
		:addRecognizer("integerDec", "^[0-9]+")
end

return PBSLexer