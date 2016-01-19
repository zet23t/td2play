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
    uint8_t width;
    uint8_t height;
    const unsigned char* data;
    uint16_t transparentColor;
    uint8_t format;
};

#endif // __LIB_IMAGE_H__
