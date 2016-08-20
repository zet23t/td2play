//#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"
#include "lib_input.h"
#include "game_main.h"


TinyScreen display = TinyScreen(TinyScreenPlus);
RenderBuffer<uint16_t,RENDER_COMMAND_COUNT> buffer;

void setup() {
    Wire.begin();
    #if defined(ARDUINO_ARCH_SAMD)
    display.begin();
    #else
    display.begin();
    #endif
    display.setFlip(0);
    display.setBrightness(8);
    display.setBitDepth(buffer.is16bit() ? 1 : 0);
    Game::setup();
}

void loop(void) {
    static unsigned long t = millis();
    ScreenButtonState::updateButtonState(display.getButtons());
    Joystick::updateJoystick();
    Game::loop();
    buffer.flush(display);
    stringBuffer.reset();
    while (millis() < t+30) continue;
    t+=30;
}
