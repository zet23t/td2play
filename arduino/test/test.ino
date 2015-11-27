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
  display.begin();
  display.setFlip(0);
  display.setBrightness(8);
  display.setBitDepth(1);
}
uint16_t rgb565[] = {0xffff,0xf,0xf0,0xffff};
Texture _tex((uint8_t*) rgb565, TextureType::rgb565sram, 2,2, 0xffff);

void loop(void) {
    static unsigned long t = millis();
    unsigned long t2 = millis();

    buffer.drawText(stringBuffer.start()->put(t2-t)->put("ms")->get(),2,2,RGB565(255,255,255), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start()->put(t2-t)->put("ms")->get(),2,22,RGB565(255,0,0), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start()->put(t2-t)->put("ms")->get(),2,32,RGB565(0,255,0), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start()->put(t2-t)->put("ms")->get(),2,42,RGB565(0,0,255), &virtualDJ_5ptFontInfo);

    static uint16_t cnt = 0;
    cnt+=1;
    buffer.drawText(stringBuffer.start()->put(cnt)->put("rgb")->get(),42,2,RGB565(cnt,cnt,cnt), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start()->put(cnt)->put("r")->get(),42,22,RGB565(cnt,0,0), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start()->put(cnt)->put("g")->get(),42,32,RGB565(0,cnt,0), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start()->put(cnt)->put("b")->get(),42,42,RGB565(0,0,cnt), &virtualDJ_5ptFontInfo);
    buffer.drawRect(60,10,8,8)->filledRect(cnt);
    buffer.drawRect(40,4,14,14)->sprite(&_tex);
    t = t2;
    buffer.flush(display);
    stringBuffer.reset();

}
