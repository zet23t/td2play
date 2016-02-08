require "gd"
local lfs = require "lfs"

local outfile = "image_data"
local outcpp = assert(io.open(outfile..".cpp","w"))
local outh = assert(io.open(outfile..".h","w"))

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