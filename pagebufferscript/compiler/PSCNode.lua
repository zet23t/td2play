local class = require "class"

local PSCNode = class()
function PSCNode:init(type, param)
	self.type = type
	self.sourceInfo = assert(param.sourceInfo)
end

PSCNode.functionDef = class(PSCNode)
function PSCNode.functionDef:init(param)
	self.Super.init(self, "functionDef", param)
	self.name = assert(param.name)
	self.returnType = assert(param.returnType)
	self.instructions = assert(param.instructions)
	self.arguments = assert(param.arguments)
end
PSCNode.functionCall = class(PSCNode)
function PSCNode.functionCall:init(param)
	self.Super.init(self, "functionCall", param)
	self.name = assert(param.name)
	self.arguments = assert(param.arguments)
end
PSCNode.value = class(PSCNode)
function PSCNode.value:init(param)
	self.Super.init(self, "value", param)
	self.valuetype = assert(param.valuetype)
	self.value = assert(param.value)
	self.isLiteral = self.value:match "^[^a-zA-Z_]"
end
PSCNode.operator = class(PSCNode)
function PSCNode.operator:init(param)
	self.Super.init(self, "operator", param)
	self.op = assert(param.op)
	self.left = assert(param.left)
	self.right = assert(param.right)
end
PSCNode.functionReturn = class(PSCNode)
function PSCNode.functionReturn:init(param)
	self.Super.init(self, "functionReturn", param)
	self.sourceInfo = param.sourceInfo
	self.expression = param.expression
  self.hasEnd = param.hasEnd
end

function PSCNode.functionDef:reduce()
	local copy = {}
	copy.sourceInfo = self.sourceInfo
	copy.name = self.name
	copy.returnType = self.returnType
	copy.instructions = {}
	copy.arguments = {}
	for i=1,#self.arguments do
		copy.arguments[i] = self.arguments[i]
	end
	for i=1,#self.instructions do 
		copy.instructions[i] = self.instructions[i]:reduce()
	end
	return self:new(copy)
end

function PSCNode.functionCall:reduce()
	local copy = {}
	copy.sourceInfo = self.sourceInfo
	copy.name = self.name
	copy.arguments = {}
	for i=1,#self.arguments do
		copy.arguments[i] = self.arguments[i]:reduce()
	end
	return self:new(copy)
end

function PSCNode.value:reduce()
	return self:new(self)
end

function PSCNode.operator:reduce()
	local copy = {}
	copy.sourceInfo = self.sourceInfo
	copy.left = self.left:reduce()
	copy.right = self.right:reduce()
	if copy.left.type == "value" and copy.left.valuetype ~= "word" 
		and copy.right.type == "value" and copy.right.valuetype ~= "word" then
		copy.type = "value"
		local l,r = copy.left.value, copy.right.value
		if self.op == "*" then
			copy.value = l * r
		elseif self.op == "+" then 
			copy.value = l + r
		elseif self.op == "/" then
			copy.value = l / r
		end
		copy.valuetype = copy.left.valuetype
		copy.sourceInfo = copy.left.sourceInfo
		copy.left, copy.right = nil
		return PSCNode.value:new(copy)
	else
		copy.type = "operator"
		copy.op = self.op
		return self:new(copy)
	end
end

function PSCNode.functionReturn:reduce()
	return self:new(self)
end

return PSCNode