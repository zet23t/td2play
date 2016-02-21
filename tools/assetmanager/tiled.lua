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

		for id, tiledata in tilesetxml:gmatch '<tile id="(%d+)">(.-)</tile>' do
			-- todo: handle tile animation information
			--	print(id,tiledata)
		end

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

	local function getobjectgroup(name) 
		local group = content:match([[<objectgroup name="]]..name..[[">(.-)</objectgroup>]])
		if group then
			local objectlist = {}
			for tags in group:gmatch "<object (.-)/>" do
				local function attribute(name)
					return tags:match(name..'="(.-)"')
				end
				local id = attribute "id"
				local type = attribute "type"
				local name = attribute "name"
				local x1 = attribute "x"
				local y1 = attribute "y"
				local width = attribute "width"
				local height = attribute "height"
				if x1 and y1 and width and height and id then
					local x2 = math.floor(x1+width+.5)
					local y2 = math.floor(y1+height+.5)
					x1 = math.floor(x1+.5)
					y1 = math.floor(y1+.5)
					local nameVal
					if type == "TRANSITION" then
						nameVal = "(const char*)"..name
					else
						nameVal = name and ('"'..name..'"') or "0"
					end
					objectlist[#objectlist+1] = ("{%d,%d,%d,%d,%s,%s}"):format(x1,y1,x2,y2,
						nameVal, type)
				end
			end
			local code = "\t\tstatic const RectObject rectObjectList[] = {\n\t\t\t"..table.concat(objectlist,",\n\t\t\t")
				.."\n\t\t};\n\t\t"
				.."static const ObjectGroup objectGroup(rectObjectList,"..(#objectlist)..");"
			return code
		else
			return nil
		end
	end

	local objectgroup = getobjectgroup "zones"

	local function to_c (tiledata)
		local out = {}
		local firstgid = tiledata.usedtileset and tiledata.usedtileset.firstgid or 0
		local rle = {}
		local current, currentCount, lineCount
		for i=1,height do
			rle[i*2-1] = 0
			rle[i*2] = 0
		end
		lineCount = 0
		local line = 1

		for i=1,#tiledata do
			local now = tiledata[i]
			if now ~= current or currentCount >=255 then
				if current then
					rle[#rle+1] = currentCount
					rle[#rle+1] = current
					lineCount = lineCount + 2
				end
				currentCount = 0
				current = now
			end
			currentCount = currentCount + 1
			if i % width == 0 then
				rle[#rle+1] = currentCount
				rle[#rle+1] = current
				currentCount,current = nil

				local low = lineCount % 256
				local high = (lineCount - low) / 256
				rle[line] = high
				rle[line+1] = low
				line = line + 2
			end
		end
		-- RLE contains run length encoded data - I'll have to write a 
		-- decoder at some point and utilize this compression technique when 
		-- needed
		--[[print(tiledata.name,#rle,#tiledata)
		for i=height*2+1,height*2+21 do
			io.write(rle[i]..", ")
		end
		print()
		for i=1,20 do
			io.write(tiledata[i]..", ")
		end
		print()]]
		for i=1,#tiledata do
			local id = tiledata[i]
			if id == 0 then id = 255 
			else id = id - firstgid end
			out[i] = ("0x%02x%s"):format(id,i%width == 0 and ",\n\t\t\t" or ",")
		end

		return table.concat(out)
	end
	local layers = {}
	local flagmapLayer
	for name in content:gmatch '<layer name="(.-)"' do
		if name ~= "flagmap" then
			layers[#layers+1] = getdata(name)
		else
			flagmapLayer = getdata(name)
		end
	end

	local filename = name

	local function tilesetname(tileset )
		return 
			tileset and 
				("ImageAsset::"..tileset.source:match "([^/]+).png$":gsub("[%- ]+","_"))
			or "0"
	end

	outhpp:write(([[
	TileMap::Scene<uint16_t> %s();
]]):format(filename))

	outfp:write("\n\tTileMap::Scene<uint16_t> "..filename.."() {\n")
	if objectgroup then
		outfp:write(objectgroup,"\n")
	end
	if flagmapLayer then
		outfp:write("\t\tstatic const uint8_t flagmapdata["..(width*height).."] PROGMEM = {\n\t\t\t"..to_c(flagmapLayer).."};\n")
		outfp:write("\t\tstatic ProgmemData flagmapLayer("..width..","..height..",flagmapdata);\n")
	end
	outfp:write("\t\tstatic const uint8_t layerdata["..#layers.."]["..(width*height).."] PROGMEM = {\n")
	for i,layer in ipairs(layers) do
		outfp:write("\t\t\t{"..to_c(layer).."},\n")
	end
	outfp:write "\t\t};\n"
	outfp:write("\t\tstatic ProgmemData layers[] = {\n")
	for i,layer in ipairs(layers) do
		outfp:write("\t\t\tProgmemData("..width..","..height..",layerdata["..(i-1).."]),\n")
	end
	outfp:write "\t\t};\n"
	outfp:write("\t\tstatic Texture<uint16_t> tilesetTextures[] = {\n")
	for i,layer in ipairs(layers) do
		outfp:write("\t\t\tTexture<uint16_t>("..tilesetname(layer.usedtileset).."),\n")
	end
	outfp:write "\t\t};\n"
	outfp:write("\t\treturn TileMap::Scene<uint16_t>(layers, "..#layers
		..",TileSet<uint16_t>(tilesetTextures, "..#layers..","..tilesizebits.."),0)")
	if flagmapLayer then
		outfp:write(".setFlagmap(flagmapLayer)")
	end
	if objectgroup then
		outfp:write(".setObjectGroup(&objectGroup)")
	end
	outfp:write ";\n"
	outfp:write "\t};\n"

	
end

for name in lfs.dir "assets" do
	if name:match "%.tmx$" then
		convertTiledXML("assets/"..name, name:match "^(.-)%.")	
	end
end

outfp:write "\n}\n"
outhpp:write "\n}\n"