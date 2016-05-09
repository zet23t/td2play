#include "font_asset.h"
namespace FontAsset {
	static const SpriteGlyph digitsGlyphs[11] = {
			{'0',0,0,6,8,6,0,0},
		{'1',6,0,6,8,6,-1,0},
		{'2',12,0,6,8,6,0,0},
		{'3',18,0,6,8,6,0,0},
		{'4',24,0,6,8,6,0,0},
		{'5',30,0,6,8,6,0,0},
		{'6',36,0,6,8,6,0,0},
		{'7',42,0,6,8,6,0,0},
		{'8',48,0,6,8,6,0,0},
		{'9',54,0,6,8,6,0,0},
		{' ',0,0,0,0,6,0,0},
	};
	const SpriteFont digits = {8,digitsGlyphs,11,&ImageAsset::digits};
	
}
