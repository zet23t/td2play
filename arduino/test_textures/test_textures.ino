/**
 * Visually testing if text output is actually working and not corrupting anything
 */
#include <SPI.h>
#include <Wire.h>
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"

TinyScreen display = TinyScreen(0);

#if defined(ARDUINO_ARCH_SAMD)
#define SCREEN_16BITS
#endif // defined

#ifndef SCREEN_16BITS
RenderBuffer<uint8_t,5> buffer;
#else
RenderBuffer<uint16_t,5> buffer;
#endif

void setup() {
    Wire.begin();
    #if defined(ARDUINO_ARCH_SAMD)
    display.begin(TinyScreenPlus);
    #else
    display.begin();
    #endif

    display.setFlip(0);
    display.setBrightness(8);
    display.setBitDepth(buffer.is16bit());
}
#ifndef SCREEN_16BITS
extern const unsigned char _image_sky_background_8bit_data[512];
extern const unsigned char _image_clouds_8bit_data[256];
Texture<uint8_t> _skyBackground((uint8_t*) _image_sky_background_8bit_data, TextureType::rgb233sram, 4,128, 0);
Texture<uint8_t> _clouds((uint8_t*) _image_clouds_8bit_data, TextureType::rgb233sram, 32, 8, 0xe3); // 0x1ff8
#else
extern const unsigned char _image_sky_background_data[256];
extern const unsigned char _image_clouds_data[512];
Texture<uint16_t> _skyBackground((uint8_t*) _image_sky_background_data, TextureType::rgb565sram, 1,128, 0);
Texture<uint16_t> _clouds((uint8_t*) _image_clouds_data, TextureType::rgb565sram, 32, 8, 0); // 0x1ff8
#endif

void loop(void) {
    static unsigned long t = millis();
    unsigned long t2 = millis();
    unsigned int n = millis() >> 5;
    int yoffset = ((int)(n>>1 & 127) - 64);
    if (yoffset < 0) yoffset = -yoffset;
    buffer.setClearBackground(false);
    buffer.drawRect(-yoffset%4,-yoffset,100,128)->sprite(&_skyBackground);
    buffer.drawRect(0,(-yoffset>>1)*0 + 40, 96, 8)->sprite(&_clouds)->blend(RenderCommandBlendMode::average);
    buffer.drawText(stringBuffer.start().put("").putDec(t2-t).put("ms").get(),
                    0,0,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);

    t = t2;
    buffer.flush(display);
    stringBuffer.reset();
}
