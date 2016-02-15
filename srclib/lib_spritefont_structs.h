#ifndef __LIB_SPRITEFONT_STRUCTS_H__
#define __LIB_SPRITEFONT_STRUCTS_H__

#include "lib_image.h"

struct SpriteGlyph {
    char letter;
    uint8_t u,v,w,h,spacing;
    int8_t offsetX, offsetY;
};

struct SpriteFont {
    int lineHeight;
    const SpriteGlyph* glyphs;
    int glyphCount;
    const ImageData* imageData;
};

#endif // __LIB_SPRITEFONT_STRUCTS_H__
