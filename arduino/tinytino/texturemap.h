#ifndef __TEXTUREMAP_H__
#define __TEXTUREMAP_H__

#ifndef PROGMEM
#define PROGMEM
#endif // PROGMEM
#include "lib_renderbuffer.h"

extern const unsigned char _image_ztiles_foreground_data[] PROGMEM;
extern const unsigned char _image_ztiles_background_data[] PROGMEM;

namespace TextureData {
    extern const Texture<uint16_t>* ztiles_background();
    extern const Texture<uint16_t>* ztiles_foreground();
}

#endif // __TEXTUREMAP_H__
