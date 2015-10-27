local class = require "class"

-------------------------------------------------------------------------------
-- The Scanner provides reading functionality for the Lexer
local Scanner = class()
function Scanner:load(file)
	local fp = assert(io.open(file,"rb"))
	self.content = fp:read "*a"
	self.pos = 1
	self.file = file
	self.line = 1
end
function Scanner:matchAndSkip(word, isRegex)
	local match
	if not isRegex and self.content:sub(self.pos,self.pos-1+#word) == word then
		match = word
	end
	if isRegex then
		match = self.content:match(word, self.pos)
	end
	if match then
		self.pos = self.pos + #match
		for i=1,#match do 
			if match:sub(i,i) == "\n" then self.line = self.line + 1 end
		end
		return match
	end
end
function Scanner:isEndOfFile()
	return self.pos > #self.content
end
function Scanner:reportPosition()
	return self.file.."@"..self.line
end
function Scanner:reportContext()
	return self.content:sub(self.pos,self.pos+5)
end
function Scanner:forward(n)
end

-------------------------------------------------------------------------------
-- The Lexer maps the Scanner's characters to tokens
local Lexer = class()
function Lexer:init(scanner)
	self.scanner = scanner or Scanner:new()
	self.keywordList = {}
	self.recognizerList = {}
end
function Lexer:next()
	if self.scanner:isEndOfFile() then 
		return nil 
	end
	for i,keyword in ipairs(self.keywordList) do
		local match = self.scanner:matchAndSkip(keyword)
		if match then return {identifier = match; match = match; line = self.scanner.line} end
	end
	for i,recognizer in ipairs(self.recognizerList) do
		local match = self.scanner:matchAndSkip(recognizer.regex, true)
		if match then return {identifier = recognizer.identifier; match = match; line = self.scanner.line} end
	end

	error("Lexer could not match a new token in "..self.scanner:reportPosition()..":\n  "..self.scanner:reportContext())
end
function Lexer:addKeywords(word)
	local function addFunc(word)
		self.keywordList[#self.keywordList + 1] = word
		return addFunc
	end
	return addFunc(word)
end
function Lexer:addRecognizer(identifier, regex)
	self.recognizerList[#self.recognizerList + 1] = {
		identifier = identifier;
		regex = regex;
	}
	return self
end


-------------------------------------------------------------------------------
-- The Parser reads Tokens from the Lexer to read sense into it
local Parser = class()
function Parser:init(lexer, defaultStatename)
	self.lexer = lexer
	self.defaultStatename = defaultStatename
	self.stateMap = {}
	self.functionMap = {}
end

function Parser:defineState(statename)
	assert(not self.stateMap[statename],"Statename '"..statename.."' already defined")
	return function (map)
		self.stateMap[statename] = map
		return self
	end
end

function Parser:getCurrentSourceInfo()
	return self.lexer.scanner:reportPosition()
end
	

function Parser:defineFunction(functionname, fn)
	assert(not self.functionMap[functionname], "Functionname '"..functionname.."' already defined")
	self.functionMap[functionname] = fn
	return self
end
function Parser:error(msg)
	error(msg.." ("..self.lexer.scanner:reportPosition()..")",2)
end
function Parser:assert(cond, msg)
	if not cond then self:error(msg) end
	return cond
end
function Parser:next()
	local match = self.current
	repeat
		self.current = self.lexer:next()
		--print(self.current and self.current.match)
	until not self.current or 
		(self.current.identifier ~= "whitespace" and self.current.identifier ~= "comment")
	--print(self.current.match)
	--if not match and self.current then return self:next() end
	return match
end
function Parser:accept(identifier)
	local match = self.current
	if not match then return nil end
	if type(identifier) == "table" then
		for i=1,#identifier do 
			if match.identifier == identifier[i] then 
				return self:next()
			end
		end
	elseif match.identifier == identifier then
		return self:next()
	end
end
function Parser:expect(identifier)
	local match = self:accept(identifier)
	if not match then 
		self:error("Expected token: "..(type(identifier) == "table" and 
				("["..table.concat(identifier,",").."]") or identifier)
			..(self.current and " current: "..self.current.match or ""))
	end
	return match
end
function Parser:load(file)
	self.lexer.scanner:load(file)
	self:next()
	return self
end	
function Parser:parse(statename,...)
	statename = statename or self.defaultStatename
	if self.functionMap[statename] then
		return self.functionMap[statename](self,...)
	end

	local map = assert(self.stateMap[statename],"No such state: "..statename)

	for match in function() return self:next() end do
		if match.identifier~="whitespace" then
			--print(match.identifier, match.match)
			local identifier = match.identifier
			while type(map[identifier]) == "string" do identifier = map[identifier] end
			if map[identifier] then 
				local result = map[identifier](self, match.identifier, match.match)
				if result then return result end
			else 
				self:error(statename.." can't handle "..match.identifier)
			end
		end
	end	
end


-------------------------------------------------------------------------------
return {
	Parser = Parser;
	Lexer = Lexer;
	Scanner = Scanner;
}