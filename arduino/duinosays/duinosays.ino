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
        return buttonState >> (4 + btn) & 1? true : false;
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

};
uint8_t ScreenButtonState::buttonState;

namespace ParticleType {
    const uint8_t bottomLeftButton = 0;
    const uint8_t topLeftButton = 1;
    const uint8_t topRightButton = 2;
    const uint8_t bottomRightButton = 3;
}

enum DuinoSaysGameMode {
    DSMODE_MAINMENU,
    DSMODE_PLAYING,
    DSMODE_GAMEOVER
};
class DuinoSays;

class Particle {
private:
    int16_t x, y, vx, vy;
    uint8_t type, frameAge, maxAge;

public:
    void init(uint8_t type, int16_t x, int16_t y, int16_t vx, int16_t vy, int8_t maxAge) {
        this->type = type;
        this->x = x;
        this->y = y;
        this->vx = vx;
        this->vy = vy;
        this->maxAge = maxAge;
        this->frameAge = 0;
    }

    bool isAlive() {
        return frameAge < maxAge;
    }

    void draw(DuinoSays *duinoSays);
};

template<uint8_t maxParticles>
class ParticleSystem {
private:
    uint8_t particleCount;
    Particle particles[maxParticles];
public:
    ParticleSystem(): particleCount(0) {};
    void draw(DuinoSays *duinoSays);
    void spawn(uint8_t type, int16_t x, int16_t y, int16_t vx, int16_t vy, uint16_t maxAge) {
        if (particleCount >= maxParticles) return;
        particles[particleCount++].init(type,x<<8,y<<8,vx,vy, maxAge);
    }
};

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
    uint16_t currentStep;
    DuinoSaysGameMode mode;
    TinyScreen display;
    Button btnTopLeft;
    Button btnTopRight;
    Button btnBottomLeft;
    Button btnBottomRight;
public:
    RenderBuffer<uint16_t,40> buffer;
    ParticleSystem<28> particleSystem;

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

    void init() {
        mode = DSMODE_MAINMENU;
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

        static unsigned long throttle = millis();
        static unsigned long t = millis();
        unsigned long t2 = millis();

        buffer.drawText(stringBuffer.start()->put(t2-t)->put("ms")->get(),2,2,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        switch (mode) {
        case DSMODE_MAINMENU: loopMainMenu(); break;
        case DSMODE_PLAYING: loopPlaying(); break;
        }

        particleSystem.draw(this);

        t = t2;
        buffer.flush(display);
        stringBuffer.reset();

        while (millis() - throttle < 50) continue;
        throttle = millis();
    }
};

void Particle::draw(DuinoSays* duinoSays) {
    const int height = 24;
    frameAge+=1;

    const uint8_t rel = 255 - frameAge * 255 / maxAge;
    const int width = 16 * rel >> 8;

    uint8_t r = 255, g = 0, b = 255;
    switch (type) {
        case ParticleType::topLeftButton: r = 255, g = 64, b = 32; break;
        case ParticleType::bottomLeftButton: r = 255, g = 255, b = 64; break;
        case ParticleType::topRightButton: r = 32, g = 255, b = 32; break;
        case ParticleType::bottomRightButton: r = 64, g = 64, b = 255; break;
    }

    duinoSays->buffer.drawRect((x>>8) - width / 2,(y>>8) - height / 2,width,height)->filledRect(duinoSays->buffer.rgb(rel * r >> 8,rel * g >> 8,rel * b >> 8));
    x+=vx;
    y+=vy;
    vx -= vx>>3;
    vy -= vy>>3;
}

template<uint8_t maxParticles>
void ParticleSystem<maxParticles>::draw(DuinoSays* duinoSays) {
    int n = 0;
    for (int i=0; i < particleCount; i += 1) {
        particles[i].draw(duinoSays);
        if (particles[i].isAlive()) {
            particles[n++] = particles[i];
        }
    }
    particleCount = n;
}

void Button::draw(DuinoSays* duinoSays) {
    const bool on = ScreenButtonState::isButtonOn(btn);
    const int width = on ? 7 : 5;
    const int height = on ? 23 : 25;
    duinoSays->buffer.drawRect(x - width / 2,y - height / 2,width,height)->filledRect(on ? pressedColor : normalColor);
    if (on && ScreenButtonState::isButtonActivated(btn)) {
        duinoSays->particleSystem.spawn(btn, x,y,x<48?2000:-2000,0, 16);
    }
}

DuinoSays duinoSays;


void setup() {
    duinoSays.setup();
}

void loop(void) {
    duinoSays.loop();
}
