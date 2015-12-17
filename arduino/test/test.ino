#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "lib_font_virtualdj.h"
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"

TinyScreen display = TinyScreen(0);
#ifdef SCREEN_16BITS
RenderBuffer<uint16_t,20> buffer;
#else
RenderBuffer<uint8_t,20> buffer;
#endif // SCREEN_16BITS

void setup() {
    Wire.begin();
    #if defined(ARDUINO_ARCH_SAMD)
    display.begin(TinyScreenPlus);
    #else
    display.begin();
    #endif
    display.setFlip(0);
    display.setBrightness(8);
    display.setBitDepth(buffer.is16bit() ? 1 : 0);
}
uint16_t rgb565[] = {
    RGB565(0,0,255),RGB565(0,0,127), RGB565(127,0,0),RGB565(255,0,0),
    RGB565(0,0,127),RGB565(0,0,127), RGB565(127,0,0),RGB565(127,0,0),
    RGB565(0,127,0),RGB565(0,127,0), 0xffff, 0xffff,
    RGB565(0,255,0),RGB565(0,127,0), 0xffff, 0xffff
};
#ifdef SCREEN_16BITS
Texture<uint16_t> _tex((uint8_t*) rgb565, TextureType::rgb565sram, 2,2, 0xffff);
#else
Texture<uint8_t> _tex((uint8_t*) rgb565, TextureType::rgb565sram, 4,4, 0xffff);
#endif

void loop(void) {
    static unsigned long t = millis();
    unsigned long t2 = millis();

    buffer.drawText(stringBuffer.start().putDec(t2-t).put("ms").get(),2,2,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start().putDec(t2-t).put("ms").get(),2,22,buffer.rgb(255,0,0), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start().putDec(t2-t).put("ms").get(),2,32,buffer.rgb(0,255,0), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start().putDec(t2-t).put("ms").get(),2,42,buffer.rgb(0,0,255), &virtualDJ_5ptFontInfo);

    static uint16_t cnt = 0;
    cnt+=1;
    buffer.drawText(stringBuffer.start().putDec(cnt).put("rgb").get(),42,2,buffer.rgb(cnt,cnt,cnt), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start().putDec(cnt).put("r").get(),42,22,buffer.rgb(cnt,0,0), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start().putDec(cnt).put("g").get(),42,32,buffer.rgb(0,cnt,0), &virtualDJ_5ptFontInfo);
    buffer.drawText(stringBuffer.start().putDec(cnt).put("b").get(),42,42,buffer.rgb(0,0,cnt), &virtualDJ_5ptFontInfo);
    buffer.drawRect(54,10,8,8)->filledRect(cnt);
    buffer.drawRect(64,10,8,8)->filledRect(buffer.rgb(128,0,0));
    buffer.drawRect(74,10,8,8)->filledRect(buffer.rgb(0,128,0));
    buffer.drawRect(84,10,8,8)->filledRect(buffer.rgb(0,0,128));
    buffer.drawRect(40,4,14,14)->sprite(&_tex);
    t = t2;
    buffer.flush(display);
    stringBuffer.reset();

}
