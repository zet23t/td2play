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
			if info.c == "\\" then info.c = [[\\]] end
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

local function packPNG (directory, outfile, config)
	local transparent = config.transparent_color
	local function crop(img)
		local w,h = img:sizeXY()
		w,h = w - 1, h - 1
		local function rgbAt(x,y)
			local col = img:getPixel(x,y)
			local r,g,b = img:red(col),img:green(col),img:blue(col)
			return r * 0x10000 + g * 0x100 + b
		end
		local function seekOpaque(start_a,end_a,start_b,end_b,swap)
			for x=start_a, end_a, start_a > end_a and -1 or 1 do
				for y=start_b, end_b, start_b > end_b and -1 or 1 do
					local col = swap and rgbAt(y,x) or rgbAt(x,y)
					if col ~= transparent then return x end
				end
			end
			return end_a - 1
		end
		local left = seekOpaque(0,w, 0,h, false)
		local right = seekOpaque(w,0, 0,h, false)
		local top = seekOpaque(0,h,0,w,true)
		local bottom = seekOpaque(h,0,0,w,true)
		--print(left,right,top,bottom)
		local crop = gd.createTrueColor(right - left+1, bottom - top+1)
		crop:copy(img,0,0,left,top,crop:sizeXY())
		--img:png("tmp1.png")
		--crop:png("tmp2.png")
		--print("save")
		return crop,left,top
	end

	local imglist = {}
	local namegroups = {}
	for filename in lfs.dir(directory) do
		if filename:match "%.png$" then
			local img = assert(gd.createFromPng(directory.."/"..filename))
			local origw,origh = img:sizeXY()
			local img,x,y = crop(img)
			assert(x and y)
			local w,h = img:sizeXY()
			local rawname,rawnumber = filename:match "^(.-)_?(%d*)%.png"
			local imginfo = {
				img = img;
				rawname = rawname;
				rawnumber = tonumber(rawnumber);
				origw = origw; origh = h;
				offsetX = x; offsetY = y;
				w = w; h = h;
				area = w * h;
				maxSide = math.max(w,h);
			}
			if imginfo.rawnumber then 
				namegroups[rawname] = namegroups[rawname] or {}
				namegroups[rawname][rawnumber + 1] = imginfo
			else
				namegroups[rawname] = {imginfo}
			end
			imglist[#imglist+1] = imginfo
		end
	end

	
	local tw,th = unpack(config.target_size)
	local freeRect = {{0,0,tw,th,tw*th}}
	local function fitRect(w,h)
		local fits = {}
		local perfect = {}
		for i=1,#freeRect do 
			local r = freeRect[i]
			r.i = i
			local rw,rh = r[3], r[4]
			if (rw == w and rh >= h) or (rh == h and rw >= w) then
				perfect[#perfect+1] = r
			elseif rw > w and rh > h then
				fits[#fits+1] = r
			end
		end
		local list = #perfect > 0 and perfect or fits
		local r = list[1]
		if r then 
			for i=2,#list do 
				local nr = list[i]
				if r[5] > nr[5] then r = nr end
			end
			table.remove(freeRect,r.i)
			local restw = r[3] - w
			local resth = r[4] - h
			local function ins(t)
				t[5] = t[3] * t[4]
				freeRect[#freeRect+1] = t
			end
			if restw == 0 and resth > 0 then
				ins{r[1],r[2]+h,w,r[4]-h}
			elseif resth == 0 and restw > 0 then
				ins{r[1]+w,r[2],r[3]-w,h}
			elseif restw > resth then
				ins{r[1]+w,r[2],r[3]-w,r[4]}
				ins{r[1],r[2]+h,w,r[4]-h}
			elseif resth >= restw then
				ins{r[1],r[2]+h,r[3],r[4]-h}
				ins{r[1]+w,r[2],r[3]-w,h}
			end
			--print(w,h,"=>",unpack(r))
			return r[1],r[2]
		else 
			error("Could not fit all images for "..directory)
		end
	end
	--table.sort(imglist,function(a,b) return a.maxSide > b.maxSide end)
	table.sort(imglist,function(a,b) return a.area > b.area end)
	local result = gd.createTrueColor(tw,th)
	result:filledRectangle(0,0,tw,th,result:colorAllocate(0,128,128))
	for i,info in ipairs(imglist) do 
		local x,y = fitRect(info.w,info.h)
		info.rect = {x,y,info.w,info.h}
		result:copy(info.img,x,y,0,0,info.w,info.h)
	end
	local basename = directory:match "_(.*)"
	local pngfile = "assets/"..basename .. ".png"
	result:png(pngfile)
	convertPNG(pngfile, config)
	os.remove(pngfile)

	outh:write(("namespace TextureAtlas_"..basename.." {\n"))
	outcpp:write(("namespace TextureAtlas_"..basename.." {\n"))
	for name, list in pairs(namegroups) do
		outh:write(("    extern const SpriteSheet %s;\n"):format(name))
		
		outcpp:write(("    const SpriteSheetRect %s_rectList[] = {\n"):format(name))
		for i=1,#list do 
			local imginfo = list[i]
			local rect = imginfo.rect

			outcpp:write(("        {%d,%d,%d,%d, %d,%d},\n"):format(rect[1],rect[2],rect[3],rect[4], imginfo.offsetX,imginfo.offsetY))
		end
		outcpp:write(("    };\n"))

		outcpp:write(("    const SpriteSheet %s = {\n"):format(name))
		outcpp:write(("         %s_rectList, %d,\n"):format(name, #list))
		outcpp:write(("    };\n"))
	end
	outh:write("}\n")
	outcpp:write("}\n")
end


local function merge (t1,t2)
	local tnew = {}
	for k,v in pairs(t1) do tnew[k] = v end
	for k,v in pairs(t2) do tnew[k] = v end
	return tnew
end

for filename in lfs.dir "assets" do
	if filename:sub(1,1) ~= '.' and filename:sub(1,4) ~= "tmp_" then
		if filename:match "^sprites_" and lfs.attributes("assets/"..filename,'mode') == "directory" then
			if lfs.attributes("assets/"..filename..".meta.lua") then
				local config = assert(dofile("assets/"..filename..".meta.lua"))
				packPNG("assets/"..filename, "assets/tmp_"..filename..".png",config)
			end
		end
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
end

outh:write [[
}

#endif
]]

outcpp:write [[
}
]]

closeFontFiles()