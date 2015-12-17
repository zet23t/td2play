local gd = require "gd"
local file = "test2.tmx"

local fp = assert(io.open(file,"r"))
local content = fp:read "*a"

local width = tonumber(content:match 'width="(%d+)"')
local height = tonumber(content:match 'height="(%d+)"')
local tilewidth = tonumber(content:match 'tilewidth="(%d+)"')
local tileheight = tonumber(content:match 'tileheight="(%d+)"')

local tilesets = {}
for tilesetxml in content:gmatch '<tileset .-</tileset>' do
	local firstgid = tilesetxml:match 'firstgid="(%d+)"'
	local src, width, height = 
		tilesetxml:match 'source="(.-)"', 
		tilesetxml:match'width="(%d+)" height="(%d+)"'
	tilesets[#tilesets+1] = {
		source = src;
		firstgid = firstgid;
		width = tonumber(width);
		height = tonumber(height);
	}
end

local function gettileset_byid(id)

end

local function getdata (layername)
	local pattern = '<layer name="'..layername
		..'".-<data encoding="csv">([%s0-9,]+)</data>'
	local csv = assert(content:match(pattern))
	local tiledata = {}
	local usedtileset
	for idStr in csv:gmatch "%d+" do 
		local id = tonumber(idStr)

		tiledata[#tiledata+1] = id
	end
	return tiledata
end

local background, foreground = getdata "background", getdata "foreground"
