local seek, repl, file = ...
local fp = assert(io.open(file,"rb"))
local content = fp:read "*a"
fp:close()
content = content:gsub(seek,repl)
fp = assert(io.open(file,"wb"))
fp:write(content)
fp:close()