return function (extends)
	local t = {}
	if extends then 
		setmetatable(t,{__index = extends}) 
		t.Super = extends
	end
	t.__mt = {__index = t}
	function t:new(...)
		local self = setmetatable({},self.__mt)
		if self.init then self:init(...) end
		return self
	end
	return t
end
