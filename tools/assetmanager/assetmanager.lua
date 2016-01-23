--[[
	This library converts assets into C code.
	Asset types:
	- PNG graphics
	- Tiled XML files

	A project contains a make_assets.lua file which requires this 
	library. It provides necessary information for converting supported assets.
]]

require "assetmanager.png2c"
require "assetmanager.tiled"