/**
 * Visually testing if text output is actually working and not corrupting anything
 */
#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "lib_font_virtualdj.h"
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"

TinyScreen display = TinyScreen(0);
RenderBuffer buffer;

void setup() {
  Wire.begin();
  SerialUSB.print("hello!");
  display.begin();
  display.setFlip(0);
  display.setBrightness(8);
  display.setBitDepth(1);
}

Texture _skyBackground((uint8_t*) _image_sky_background_data, TextureType::rgb565sram, 1,128, 0);
Texture _clouds((uint8_t*) _image_clouds_data, TextureType::rgb565sram, 64, 8, 0x1ff8);

void loop(void) {
    unsigned int n = millis() >> 5;
    int yoffset = ((int)(n>>1 & 127) - 64);
    if (yoffset < 0) yoffset = -yoffset;
    buffer.drawRect(0,-yoffset,96,128)->sprite(&_skyBackground);
    buffer.drawRect(0,(-yoffset>>1) + 80, 96, 8)->sprite(&_clouds);
   // buffer.drawText("rgb-565 sram",2,52,RGB565(255,255,255), &virtualDJ_5ptFontInfo);

    buffer.flush(display);
}
