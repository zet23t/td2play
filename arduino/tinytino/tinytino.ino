#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "lib_font_virtualdj.h"
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"

#include "game.h"

static TinyScreen display(0);
RenderBuffer<uint16_t,40> renderBuffer;

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

    //renderBuffer.drawRect(39,35,5,5)->filledRect(renderBuffer.rgb(255,255,255));

    game.update();

    renderBuffer.flush(display);
    stringBuffer.reset();

    while (millis() - throttle < 50) continue;
    throttle = millis();
}
