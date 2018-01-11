local seek, repl, from, to = ...
local fp = assert(io.open(from,"rb"))
local content = fp:read "*a"
fp:close()

content = content:gsub(seek,repl)

local cmpfp = io.open(to, "rb")
if cmpfp then
	local cmp = cmpfp:read "*a"
	if cmp:gsub("%s+"," ") == content:gsub("%s+", " ") then 
		return
	end
	print("Diff", to)
	local lineA = cmp:gmatch "[^\n]+"
	local lineB = content:gmatch "[^\n]+"
	while true do
		local la = lineA()
		local lb = lineB()
		if la ~=lb then print(la.."\n"..lb) end
		if not la or not lb then break end
	end
--	print(cmp,content)
	cmpfp:close()
end
print "updating file"
fp = assert(io.open(to,"wb"))
fp:write(content)
fp:close()