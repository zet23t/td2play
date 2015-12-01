#include <TinyScreen.h>
#include <SPI.h>
#include <Wire.h>
#include "lib_font_virtualdj.h"
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"

enum ScreenButtonId {
    SCREENBUTTON_BOTTOMLEFT = 0,
    SCREENBUTTON_TOPLEFT = 1,
    SCREENBUTTON_TOPRIGHT = 2,
    SCREENBUTTON_BOTTOMRIGHT = 3
};
class ScreenButtonState {
private:
    static uint8_t buttonState;
public:
    static void updateButtonState(uint8_t newState) {
        buttonState = buttonState << 4 | newState;
    }
    static bool isButtonOn(ScreenButtonId btn) {
        return buttonState >> btn & 1 ? true : false;
    }
    static bool wasButtonOn(ScreenButtonId btn) {
        return buttonState >> (4 + btn) ? true : false;
    }
    static bool isButtonActivated(ScreenButtonId btn) {
        return isButtonOn(btn) && !wasButtonOn(btn);
    }
    static bool wasButtonReleased(ScreenButtonId btn) {
        return !isButtonOn(btn) && wasButtonOn(btn);
    }
    static bool isAnyButtonOn() {
        return buttonState & 15 ? true : false;
    }
    static bool wasAnyButtonOn() {
        return buttonState & 0xf0 ? true : false;
    }
    static bool isAnyButtonActivated() {
        return isAnyButtonOn() && !wasAnyButtonOn();
    }
    static bool wasAnyButtonReleased() {
        return !isAnyButtonOn() && wasAnyButtonOn();
    }

    static bool isBottomLeftPressed() {
        return buttonState & 1 ? true : false;
    }
    static bool isTopLeftPressed() {
        return buttonState & 2 ? true : false;
    }
    static bool isTopRightPressed() {
        return buttonState & 4 ? true : false;
    }
    static bool isBottomRightPressed() {
        return buttonState & 8 ? true : false;
    }
    static bool wasBottomLeftPressed() {
        return buttonState & 16 ? true : false;
    }
    static bool wasTopLeftPressed () {
        return buttonState & 32 ? true : false;
    }
    static bool wasTopRightPressed() {
        return buttonState & 64 ? true : false;
    }
    static bool wasBottomRightPressed() {
        return buttonState & 128 ? true : false;
    }

    static bool wasBottomLeftActivated() {
        return !wasBottomLeftPressed() && isBottomLeftPressed();
    }
    static bool wasTopLeftActivated() {
        return !wasTopLeftPressed() && isTopLeftPressed();
    }
    static bool wasBottomRightActivated() {
        return !wasBottomRightPressed() && isBottomRightPressed();
    }
    static bool wasTopRightActivated() {
        return !wasTopRightPressed() && isTopRightPressed();
    }

    static bool wasBottomLeftReleased() {
        return wasBottomLeftPressed() && !isBottomLeftPressed();
    }
    static bool wasTopLeftReleased() {
        return wasTopLeftPressed() && !isTopLeftPressed();
    }
    static bool wasBottomRightReleased() {
        return wasBottomRightPressed() && !isBottomRightPressed();
    }
    static bool wasTopRightReleased() {
        return wasTopRightPressed() && !isTopRightPressed();
    }

};
uint8_t ScreenButtonState::buttonState;

enum DuinoSaysGameMode {
    DSMODE_MAINMENU,
    DSMODE_PLAYING,
    DSMODE_GAMEOVER
};
class DuinoSays;

class Button {
private:
    uint8_t x;
    uint8_t y;
    ScreenButtonId btn;
    uint16_t normalColor;
    uint16_t pressedColor;
public:
    Button(ScreenButtonId btn, uint8_t x, uint8_t y, uint16_t normalColor, uint16_t pressedColor):
        x(x), y(y), btn(btn), normalColor(normalColor), pressedColor(pressedColor) {};
    void draw(DuinoSays *duinoSays);
};


class DuinoSays {
private:
    uint16_t seed;
    uint16_t currentLevel;
    DuinoSaysGameMode mode;
    TinyScreen display;
    Button btnTopLeft;
    Button btnTopRight;
    Button btnBottomLeft;
    Button btnBottomRight;
public:
    RenderBuffer<uint16_t,8> buffer;

public:
    DuinoSays():
        mode(DSMODE_MAINMENU),
        display(0),
        btnTopLeft(SCREENBUTTON_TOPLEFT, 2, 16, buffer.rgb(192,0,0), buffer.rgb(255,0,0)),
        btnTopRight(SCREENBUTTON_TOPRIGHT, 94, 16, buffer.rgb(0,192,0), buffer.rgb(0,255,0)),
        btnBottomLeft(SCREENBUTTON_BOTTOMLEFT, 2, 48, buffer.rgb(192,192,0), buffer.rgb(255,255,0)),
        btnBottomRight(SCREENBUTTON_BOTTOMRIGHT, 94, 48, buffer.rgb(0,0,192), buffer.rgb(64,64,255))
    {

    }

    void setup() {
        Wire.begin();
        display.begin();
        display.setFlip(0);
        display.setBrightness(8);
        display.setBitDepth(buffer.is16bit()? 1:0);
    }

    void drawButton(bool on, uint16_t normalColor, uint16_t pressedColor, uint8_t x, uint8_t y) {
        const int width = on ? 7 : 5;
        const int height = on ? 23 : 25;
        buffer.drawRect(x - width / 2,y - height / 2,width,height)->filledRect(on ? pressedColor : normalColor);
    }

    void drawButtons() {
        //drawButton(ScreenButtonState::isButtonActivated(SCREENBUTTON_TOPLEFT),buffer.rgb(192,0,0),buffer.rgb(255,0,0), 2,16);
        btnTopLeft.draw(this);
        btnTopRight.draw(this);
        btnBottomLeft.draw(this);
        btnBottomRight.draw(this);
    }

    void loopMainMenu() {
        buffer.drawText(stringBuffer.start()->load(PSTR("duino says ..."))->get(),16,28,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("push button"))->get(),16,45,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("to <start>"))->get(),20,55,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        drawButtons();
        if (ScreenButtonState::wasAnyButtonReleased())
        {
            switchToPlaying();
        }
    }

    void switchToPlaying() {
        mode = DSMODE_PLAYING;
        seed = millis();
    }

    void loopPlaying() {
        drawButtons();
    }

    void loop() {
        ScreenButtonState::updateButtonState(display.getButtons());


        static unsigned long t = millis();
        unsigned long t2 = millis();

        buffer.drawText(stringBuffer.start()->put(t2-t)->put("ms")->get(),2,2,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        switch (mode) {
        case DSMODE_MAINMENU: loopMainMenu(); break;
        case DSMODE_PLAYING: loopPlaying(); break;
        }

        t = t2;
        buffer.flush(display);
        stringBuffer.reset();
    }
};

void Button::draw(DuinoSays* duinoSays) {
    const bool on = ScreenButtonState::isButtonOn(btn);
    const int width = on ? 7 : 5;
    const int height = on ? 23 : 25;
    duinoSays->buffer.drawRect(x - width / 2,y - height / 2,width,height)->filledRect(on ? pressedColor : normalColor);
}

DuinoSays duinoSays;


void setup() {
    duinoSays.setup();
}

void loop(void) {
    duinoSays.loop();
}
