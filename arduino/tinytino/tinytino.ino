#include <SPI.h>
#include <Wire.h>
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"

#include "game.h"

static TinyScreen display(0);
RenderBuffer<uint16_t,128> renderBuffer;

Game game;

void setup() {
    Wire.begin();
    #if defined(ARDUINO_ARCH_SAMD)
    display.begin(TinyScreenPlus);
    #else
    display.begin();
    #endif
    display.setFlip(0);
    display.setBrightness(8);
    display.setBitDepth(1);
}

void loop() {
    static unsigned long throttle = millis();
    static unsigned long t = 0;
    unsigned long t0 = millis();

    game.update();

//    renderBuffer.drawText(stringBuffer.start().putDec(t).put("ms").get(),2,2,renderBuffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);


    renderBuffer.flush(display);
    stringBuffer.reset();
    t = millis() - t0;

    while (millis() - throttle < 50) continue;
    throttle = millis();
}
