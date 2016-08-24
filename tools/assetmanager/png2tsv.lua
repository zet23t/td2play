-- shameless copy paste from png2c.lua
local gd = require "gd"
local lfs = require "lfs"
local function convertPNG(pngfile, outfile)
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

	local function rgbToBytes(img, col, as_short)
		local short = rgbToRGB565(img, col)
		local high = floor(short / 256)
		local low = short % 256
		local str = (string.char(high,low))
		return str
	end

	
	local width, height = img:sizeXY()
	local name = pngfile:gsub("^.-([^/\\]+)%.png","%1"):gsub("%-","_")
	local pixels = {}

	local fp = assert(io.open(outfile,"wb"))
	fp:seek("set",0)
	local n = 0
	for y=0,height-1 do
		for x=0,width-1 do
			fp:write((rgbToBytes(img,img:getPixel(x,y))))
			n = n + 2
		end
	end
	fp:close()
end

convertPNG(...)