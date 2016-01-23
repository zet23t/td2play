-- this script parses a tiled saved level and converts it into a C encoded 
-- data structure that can be compiled.

local gd = require "gd"
local transform = require "assetmanager.transform"

local outfp = assert(io.open("asset_tilemap.cpp","wb"))
outfp:write([[
// This file is generated from a tiled tilemap!
#include <inttypes.h>
#include "image_data.h"
#include "asset_tilemap.h"

namespace TilemapAsset {
	using namespace TileMap;
]])

local outhpp = assert(io.open("asset_tilemap.h","wb"))
outhpp:write [[
// This file is generated from a tiled tilemap!
#include "lib_tilemap.h"

namespace TilemapAsset {
]]

function convertTiledXML(path, name)
	outfp:write("// "..path)
	print("Converting "..path)
	local file = path

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
		local csv = assert(content:match(pattern),"No layer with name "..layername.." found")
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
		local firstgid = tiledata.usedtileset and tiledata.usedtileset.firstgid or 0

		for i=1,#tiledata do
			local id = tiledata[i]
			if id == 0 then id = 255 
			else id = id - firstgid end
			out[i] = ("0x%02x%s"):format(id,i%width == 0 and ",\n\t\t\t" or ",")
		end

		return table.concat(out)
	end

	local background, foreground = getdata "background", getdata "foreground"
	local filename = name

	local function tilesetname(tileset )
		return 
			tileset and 
				("ImageAsset::"..tileset.source:match "([^/]+).png$":gsub("[%- ]+","_"))
			or "0"
	end

	outhpp:write(([[
	TileMap::SceneBgFg<uint16_t> %s();
]]):format(filename))

	outfp:write(transform([[
	
	TileMap::SceneBgFg<uint16_t> {$name}() {
		static const uint8_t {$name}_foreground[] PROGMEM = {
			{$data_foreground}
		};
		static const uint8_t {$name}_background[] PROGMEM = {
			{$data_background}
		};
		const ProgmemData foreground = ProgmemData({$width}, {$height}, {$name}_foreground);
		const ProgmemData background = ProgmemData({$width}, {$height}, {$name}_background);

		return TileMap::SceneBgFg<uint16_t>(
			background, foreground,
			TileSetBgFg<uint16_t>(
				Texture<uint16_t>({$tileset_background_name}), 
				Texture<uint16_t>({$tileset_foreground_name}), 
				{$tilesizebits}
			), 
			0
		);
	}
	]], {
		name = filename, 
		data_foreground = to_c(foreground),
		data_background = to_c(background),
		tileset_background_name = tilesetname(background.usedtileset), 
		tileset_foreground_name = tilesetname(foreground.usedtileset), 
		width = width, height = height, tilesizebits = tilesizebits
	}))
end

for name in lfs.dir "assets" do
	if name:match "%.tmx$" then
		convertTiledXML("assets/"..name, name:match "^(.-)%.")	
	end
end

outfp:write "\n}\n"
outhpp:write "\n}\n"