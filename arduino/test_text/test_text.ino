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
RenderBuffer<uint8_t> buffer;

void setup() {
  Wire.begin();
  display.begin();
  display.setFlip(0);
  display.setBrightness(8);
  display.setBitDepth(0);
}

void loop(void) {
    static unsigned int n = 0;
    buffer.drawText("hello world",2,-2,RGB565(255,255,255), &virtualDJ_5ptFontInfo);
    buffer.drawText("hello world",-10,12,RGB565(255,255,255), &virtualDJ_5ptFontInfo);
    buffer.drawText("hello world",80,22,RGB565(255,255,255), &virtualDJ_5ptFontInfo);

    buffer.drawText("hello world",-20 + ((n++ >> 3) % 128),32,RGB565(255,255,255), &virtualDJ_5ptFontInfo);
    buffer.drawText("hello world",100 + -((n++ >> 3) % 128),60,RGB565(255,255,255), &virtualDJ_5ptFontInfo);

    buffer.flush(display);
}
