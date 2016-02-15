return function (self,values)
	assert(values)
	return (self:gsub("{$([a-zA-Z_][a-zA-z0-9_]*):?([0-9a-zA-Z]*)}",function(x,fmt) 
		local v = assert(values[x],"no such key: "..x)
		if #fmt > 0 then return ("%"..fmt):format(values[x]) end
		return values[x]
	end))
end

--print(("hello {$foobar}"):transform{foobar = "world"})
--print(("account {$total:03x}"):transform{total = 12})