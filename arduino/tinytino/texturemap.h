#ifndef __TEXTUREMAP_H__
#define __TEXTUREMAP_H__


extern const unsigned char _image_ztiles_foreground_data[] PROGMEM;
extern const unsigned char _image_ztiles_background_data[] PROGMEM;

namespace TextureData {
    extern const Texture<uint16_t> ztiles_background(_image_ztiles_background_data, TextureType::rgb565progmem, 128, 64, 0);
    extern const Texture<uint16_t> ztiles_foreground(_image_ztiles_foreground_data, TextureType::rgb565progmem, 128, 64, 0);
}

#endif // __TEXTUREMAP_H__
