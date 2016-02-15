-- this file desires to be rewritten ...

require "gd"
local lfs = require "lfs"
local transform = require "assetmanager.transform"

local outfile = "image_data"
local outcpp = assert(io.open(outfile..".cpp","w"))
local outh = assert(io.open(outfile..".h","w"))

local outFontH, outFontCpp
local function writeFontHeader(...)
	if not outFontH then
		outFontH = assert(io.open("font_asset.h","w"))
		outFontH:write[[
/* auto generated file! */
#ifndef __FONT_ASSET_H__
#define __FONT_ASSET_H__
#include "image_data.h"
#include "lib_spritefont.h"
namespace FontAsset {
]]
	end
	return outFontH:write(...)
end
local function closeFontFiles()
	if outFontH then
		outFontH:write "\n}\n#endif\n"
		outFontH:close()
		outFontH = nil
	end
	if outFontCpp then
		outFontCpp:write "\n}\n"
		outFontCpp:close()
		outFontCpp = nil
	end
end

local function writeFontCpp(...)
	if not outFontCpp then
		outFontCpp = assert(io.open("font_asset.cpp","w"))
		outFontCpp:write [[
#include "font_asset.h"
namespace FontAsset {
]]
	end
	return outFontCpp:write(...)
end

local function writeFontMap(imagename, fontmap)
	writeFontHeader(([[
	extern const SpriteFont %s;
	]]):format(imagename))
	local n = #fontmap
	local lineheight = fontmap.lineheight or 8
	writeFontCpp(transform([[
	static const SpriteGlyph {$name}Glyphs[{$n}] = {
	]],{name=imagename,n=n}))
	for i=1,n do
		local info = fontmap[i]
		if #info.c ~= 1 then
			print("Warning, non-char in fontmap: "..info.c)
			writeFontCpp(([[
		{0,0,0,0,0,0,0,0},
]]));
		else
			assert(#info.c == 1)
			if info.c == "'" then info.c = "\\'" end
			writeFontCpp(transform([[
		{'{$c}',{$u},{$v},{$w},{$h},{$spacing},{$offsetX},{$offsetY}},
]], info));
		end
	end
	writeFontCpp(transform([[
	};
	const SpriteFont {$name} = {{$lineheight},{$name}Glyphs,{$glyphCount},&ImageAsset::{$name}};
	]],{name=imagename;glyphCount=n,lineheight=lineheight}))
end

outh:write [[
/* auto generated image file! */
#ifndef __IMAGE_DATA_H__
#define __IMAGE_DATA_H__
#include "lib_image.h"

namespace ImageAsset {
]]

outcpp:write [[
/* auto generated image file! */
#include "image_data.h"

namespace ImageAsset {
]]

local function convertPNG(pngfile, config)
	print("converting "..pngfile.." to "..config.format)
	local img = assert(gd.createFromPng(pngfile))
	local floor = math.floor
	local min = math.min
	local byteCount = 0

	local function rgbToRGB565(img, col)
		local r = min(31,floor(img:red(col)/255   * 31 +.25))
		local g = min(63,floor(img:green(col)/255 * 63 +.25))
		local b = min(31,floor(img:blue(col)/255  * 31 +.25))
		return r + g * 32 + b * 32 * 64
	end

	local function rgbToRGB233(img,col)
		local r = min(3,floor(img:red(col)/255 * 3+.25))
		local g = min(7,floor(img:green(col)/255 * 7+.25))
		local b = min(7,floor(img:blue(col)/255 * 7+.25))
		return r + g * 4 + b * 32
	end

	local function rgbToString233(img, col)
		byteCount = byteCount + 1
		return ("0x%x"):format(rgbToRGB233(img,col))
	end

	local function rgbToString565(img, col, as_short, no_count)
		if not no_count then
			byteCount = byteCount + 2
		end

		local short = rgbToRGB565(img, col)
		local high = floor(short / 256)
		local low = short % 256
		if as_short then
			return ("0x%02x%02x"):format(low,high)
		else
			return ("0x%02x,0x%02x"):format( high, low)
		end
	end

	local rgbToString

	if config.format == "rgb233" then
		rgbToString = rgbToString233
	elseif config.format == "rgb565" then
		rgbToString = rgbToString565
	else
		error("File "..pngfile.." specifies unknown format: "..config.format)
	end

	local width, height = img:sizeXY()
	local name = pngfile:gsub("^.-([^/\\]+)%.png","%1"):gsub("%-","_")
	local pixels = {}
	for y=0,height-1 do
		for x=0,width-1 do
			pixels[#pixels+1] = rgbToString(img,img:getPixel(x,y))
		end
		pixels[#pixels] = pixels[#pixels].."\n"
	end
	outcpp:write(([[

const unsigned char %s_data[] PROGMEM = {
	%s
};
const ImageData %s = { %d,%d, %s_data,%s, ImageFormat::%s };

]]):format(name,table.concat(pixels,","),
		name,width,height,name,
		(img:getTransparent() and rgbToByte(img,img:getTransparent()))
			or (config.transparent_color and rgbToString(img,config.transparent_color,true, true)) or -1,
		config.format:upper()
	))
	outh:write(([[
extern const unsigned char %s_data[%d] PROGMEM;
extern const ImageData %s;
]]):format(name, byteCount, name))

	if config.fontmap then
		writeFontMap(name, config.fontmap)
	end
end

local function merge (t1,t2)
	local tnew = {}
	for k,v in pairs(t1) do tnew[k] = v end
	for k,v in pairs(t2) do tnew[k] = v end
	return tnew
end

for filename in lfs.dir "assets" do
	if filename:match "%.png$" then
		local config = {
			format = "rgb233";
		}
		if lfs.attributes("assets/"..filename..".meta.lua") then
			config = merge(config, assert(dofile("assets/"..filename..".meta.lua")))
		end
		convertPNG("assets/"..filename, config)
	end
end

outh:write [[
}

#endif
]]

outcpp:write [[
}
]]

closeFontFiles()