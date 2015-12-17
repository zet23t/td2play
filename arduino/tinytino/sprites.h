#ifndef __SPRITES_H__
#define __SPRITES_H__

#ifndef PROGMEM
#define PROGMEM
#endif // PROGMEM

extern const unsigned char _image_beastlands[] PROGMEM;
namespace texture {
    extern const Texture<uint16_t> beastlands;
}
#endif // __SPRITES_H__
