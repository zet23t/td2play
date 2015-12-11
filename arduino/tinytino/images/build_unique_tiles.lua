local gd = require "gd"
local file = "beastlands.png"
local tileDims = {x=8,y=8}


local img = gd.createFromPng(file)
assert(img:sizeX() % tileDims.x == 0)
assert(img:sizeY() % tileDims.y == 0)

local tileMap = {}
local tileImageList = {}
function extractTile(tileX,tileY)
	local x = tileX * tileDims.x
	local y = tileY * tileDims.y
	local tile = gd.createTrueColor(tileDims.x, tileDims.y)
	tile:copy(img, 0,0, x, y, tileDims.x, tileDims.y)
	local str = tile:pngStr()
	if tileMap[str] then return end
	tileMap[str] = true
	tileImageList[#tileImageList + 1] = tile
end

for tileY = 0, img:sizeY() / tileDims.y - 1 do
	for tileX = 0, img:sizeX() / tileDims.x - 1 do
		extractTile(tileX, tileY)
	end
end

local tileCount = #tileImageList
print("Found "..#tileImageList.." unique tiles")
local tileNx = math.ceil(math.sqrt(tileCount))
local tileNy = math.ceil(tileCount / tileNx)
local out = gd.createTrueColor(tileDims.x * tileNx, tileDims.y * tileNy)
for i=1,tileCount do
	local x = (i-1) % tileNx
	local y = (i-1-x) / tileNx * tileDims.y
	x = x * tileDims.x
	out:copy(tileImageList[i],x,y,0,0,tileDims.x, tileDims.y)
end
out:png(file:match "(.*)%.png" .. "-tiled.png")