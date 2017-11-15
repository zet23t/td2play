#ifndef __LIB_IMAGE_H__
#define __LIB_IMAGE_H__

#include <inttypes.h>

#ifndef PROGMEM
#define PROGMEM
#endif

namespace ImageFormat {
    const uint8_t RGB233 = 4;
    const uint8_t RGB565 = 2;
};

struct ImageData {
    uint16_t width;
    uint16_t height;
    const unsigned char* data;
    uint16_t transparentColor;
    uint8_t format;
};

struct SpriteSheetRect {
    uint8_t x, y, width, height, offsetX, offsetY,origWidth,origHeight;
};

struct SpriteSheet {
    const SpriteSheetRect *sprites;
    uint8_t spriteCount;
};

#endif // __LIB_IMAGE_H__
