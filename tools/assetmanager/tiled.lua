-- this script parses a tiled saved level and converts it into a C encoded 
-- data structure that can be compiled.

local gd = require "gd"
local file = "test2.tmx"

local fp = assert(io.open(file,"r"))
local content = fp:read "*a"

local width = tonumber(content:match 'width="(%d+)"')
local height = tonumber(content:match 'height="(%d+)"')
local tilewidth = tonumber(content:match 'tilewidth="(%d+)"')
local tileheight = tonumber(content:match 'tileheight="(%d+)"')
assert(tilewidth == tileheight, "tile size must be square")

local tilesizebits
for i=1,5 do 
	if 2^i == tilewidth then
		tilesizebits = i
		break
	end
end
assert(tilesizebits, "tile size must be power of 2") 

local tilesets = {}
local tileset_by_id = {}
for tilesetxml in content:gmatch '<tileset .-</tileset>' do
	local tilecount = tonumber(tilesetxml:match 'tilecount="(%d+)"')
	local firstgid = tilesetxml:match 'firstgid="(%d+)"'
	local src, width, height = 
		tilesetxml:match 'source="(.-)"', 
		tilesetxml:match'width="(%d+)" height="(%d+)"'
	local tset = {
		source = src;
		firstgid = firstgid;
		width = tonumber(width);
		height = tonumber(height);
		tilecount = tilecount;
	}
	tilesets[#tilesets+1] = tset
	for i=0,tilecount - 1 do
		assert(i + firstgid ~= 0)
		tileset_by_id[i + firstgid] = tset
	end
end

local function gettileset_by_id(id)
	return tileset_by_id[id]
end

local function getdata (layername)
	local pattern = '<layer name="'..layername
		..'".-<data encoding="csv">([%s0-9,]+)</data>'
	local csv = assert(content:match(pattern))
	local tiledata = {name = layername}
	local usedtileset, firstuse
	for idStr in csv:gmatch "%d+" do 
		local id = tonumber(idStr)
		local tset = gettileset_by_id(id)
		if usedtileset ~= tset and tset then
			local x,y = #tiledata % width, math.floor(#tiledata / width)
			if usedtileset then
				error("Layer "..layername.." uses more than one tileset @ "
						..x..","..y..": "
						..usedtileset.source.." first used @ "..table.concat(firstuse,", ")
						.." vs "..tset.source.."!")
			elseif not usedtileset then
				firstuse = {x,y}
				usedtileset = tset
			end
		end
		tiledata[#tiledata+1] = id
	end
	tiledata.usedtileset = usedtileset
	return tiledata
end

local function to_c (tiledata)
	local out = {}
	local firstgid = tiledata.usedtileset.firstgid

	for i=1,#tiledata do
		local id = tiledata[i]
		if id == 0 then id = 255 
		else id = id - firstgid end
		out[i] = ("0x%02x%s"):format(id,i%width == 0 and ",\n  " or ",")
	end

	return table.concat(out)
end

local background, foreground = getdata "background", getdata "foreground"
local filename = file:match "[^%.]+"

local outfp = assert(io.open(filename..".cpp","wb"))
outfp:write(([[
// This file is generated from a tiled tilemap!
#include <inttypes.h>
#include "lib_tilemap.h"
#include "texturemap.h"

static const uint8_t _foreground[] PROGMEM = {
  %s};
static const uint8_t _background[] PROGMEM = {
  %s};
TileDataMap %s = TileDataMap(
  TextureData::%s, TextureData::%s, %d, %d, %d, _background, _foreground
);
]]):format(
	to_c(foreground), to_c(background),
	-- tiledatamap
	file:match "[^%.]+", 
	background.usedtileset.source:match "([^/]+).png$":gsub("[%- ]+","_"), 
	foreground.usedtileset.source:match "([^/]+).png$":gsub("[%- ]+","_"), 
	width, height, tilesizebits
))
