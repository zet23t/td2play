local gd = require "gd"
local file = "test.tmx"
local fp = assert(io.open(file,"r"))
local content = fp:read "*a"

local width = tonumber(content:match 'width="(%d+)"')
local height = tonumber(content:match 'height="(%d+)"')
local tilewidth = tonumber(content:match 'tilewidth="(%d+)"')
local tileheight = tonumber(content:match 'tileheight="(%d+)"')
local csv = content:match '<data encoding="csv">([%s0-9,]+)</data>'
local tiledata = {}
for id in csv:gmatch "%d+" do tiledata[#tiledata+1] = tonumber(id) end

local imagefile = content:match '<image source="(.-)"'
local img = gd.createFromPng(imagefile)

local tileMap = {}
local tileImageList = {}
local idMapping = {}
local tileCountX = img:sizeX() / tilewidth
print("tile count x: ",tileCountX)
function extractTile(tileX,tileY)
	local x = tileX * tilewidth
	local y = tileY * tileheight
	local id = tileX + tileY * tileCountX + 1
	local tile = gd.createTrueColor(tilewidth, tileheight)
	tile:copy(img, 0,0, x, y, tilewidth, tileheight)
	local str = tile:pngStr()
	if id == 596 then print(x,y,tileX,tileY) end
	if tileMap[str] then 
		idMapping[id] = tileMap[str]
	else
		local tileListId = #tileImageList + 1
		idMapping[id] = tileListId
		tileMap[str] = tileListId
		tileImageList[tileListId] = {img = tile, src = {x,y}, origId = id}
	end
end

for tileY = 0, img:sizeY() / tileheight - 1 do
	for tileX = 0, tileCountX - 1 do
		extractTile(tileX, tileY)
	end
end

local usedTileList = {}
local mappedTiledata = {}
for i=1,#tiledata do 
	local tileId = tiledata[i]
	tileListId = assert(idMapping[tileId])
	tile = tileImageList[tileListId]
	if not tile.newId then
		tile.newId = #usedTileList + 1
		usedTileList[#usedTileList + 1] = tile
	end
	if i <= 5 then 
		print(i,tile.origId, tileId, tileListId, unpack(tile.src)) end
	mappedTiledata[i] = tile.newId - 1
end


-- write tile image
local tileCount = #usedTileList
print("Found "..#usedTileList.." unique tiles")
local function nextPwr2(n)
	local x = 1
	while x < n do x = x * 2 end
	return x
end
local tileNx = nextPwr2(math.ceil(math.sqrt(tileCount)))
local tileNy = nextPwr2(math.ceil(tileCount / tileNx))
local out = gd.createTrueColor(tilewidth * tileNx, tileheight * tileNy)
for i=1,tileCount do
	local tile = usedTileList[i]
	local x = (i-1) % tileNx
	local y = (i-1-x) / tileNx * tileheight
	x = x * tilewidth
	out:copy(tile.img,x,y,0,0,tilewidth, tileheight)
end
out:png(imagefile:match "(.*)%.png" .. "-tiled.png")

-- write tile map data
local outfile = "../data_level.cpp"
local outfp = io.open(outfile,"wb")
outfp:write [[
// autogenerated level file
#include <inttypes.h>
uint16_t data_level[] = {
]]
for y=0,height-1 do
	for x=0,width-1 do
		local index = x + y * width + 1
		outfp:write(mappedTiledata[index]..",")
	end
	outfp:write "\n"
end
outfp:write [[
};
]]