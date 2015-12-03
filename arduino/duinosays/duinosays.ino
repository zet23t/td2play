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
    bool isEnabled;
    bool isPressed;
public:
    Button(ScreenButtonId btn, uint8_t x, uint8_t y, uint16_t normalColor, uint16_t pressedColor):
        x(x), y(y), btn(btn), normalColor(normalColor), pressedColor(pressedColor), isEnabled(true) {};
    void setEnabled(bool on) {
        isEnabled = on;
    }
    uint16_t getNormalColor() {
        return normalColor;
    }
    bool getIsPressed() {
        return isPressed;
    }
    void draw(DuinoSays *duinoSays);
};


class DuinoSays {
private:
    uint16_t seed;
    uint16_t currentLevel;
    uint16_t currentStep;
    uint16_t progressCounter;
    bool isShowingColors;
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

    Button* getButton(uint8_t n) {
        switch (n%4) {
        case 0: return &btnTopLeft;
        case 1: return &btnTopRight;
        case 2: return &btnBottomLeft;
        case 3: return &btnBottomRight;
        }
        return 0;
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

    void nextLevel() {
        currentLevel += 1;
        progressCounter = 0;
        currentStep = 0;
        isShowingColors = true;
        btnTopLeft.setEnabled(false);
        btnTopRight.setEnabled(false);
        btnBottomLeft.setEnabled(false);
        btnBottomRight.setEnabled(false);
    }

    void switchToPlaying() {
        mode = DSMODE_PLAYING;
        currentLevel = 0;
        seed = millis();
        nextLevel();
    }

    void switchToGameOver() {
        mode = DSMODE_GAMEOVER;
        progressCounter = 0;
    }


    void switchToMainMenu() {
        mode = DSMODE_MAINMENU;
        setButtonsEnabled(true);
    }

    void setButtonsEnabled(bool enabled) {
        btnTopLeft.setEnabled(enabled);
        btnTopRight.setEnabled(enabled);
        btnBottomLeft.setEnabled(enabled);
        btnBottomRight.setEnabled(enabled);
    }

    uint8_t getColor(uint16_t n) const {
        uint16_t a = seed;
        uint16_t b = seed ^ 0x2945;
        uint16_t c = seed << 8 | seed >> 8;
        while (n-- > 0) {
            a = a << 2 ^ b >> 3 ^ c << 2;
            b = a >> 4 ^ b >> 5 ^ c >> 4;
            c = a >> 1 ^ b >> 1 ^ c >> 6;
        }
        return c % 4;
    }

    void loopPlaying() {
        drawButtons();
        if (isShowingColors) {
            if (currentStep == 0) {
                buffer.drawText(stringBuffer.start()->load(PSTR("duino says ..."))->get(),16,28,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
            }
            if (currentStep <= currentLevel && currentStep > 0) {
                const uint8_t w = 4 + progressCounter, h = 4 + progressCounter;
                uint8_t col = getColor(currentStep);
                Button *button = getButton(col);
                buffer.drawRect(48-w/2,32-h/2,w,h)->filledRect(button->getNormalColor());
                buffer.drawText(stringBuffer.start()->put(currentStep)->get(),44,52,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
            }
            if (progressCounter++ > 30) {
                currentStep += 1;
                progressCounter = 0;
                if (currentStep > currentLevel) {
                    isShowingColors = false;
                    setButtonsEnabled(true);
                    currentStep = 1;
                }
            }
        } else {
            uint8_t col = getColor(currentStep);
            for (uint8_t i=0;i<4;i+=1) {
                Button *button = getButton(i);
                if (button->getIsPressed()) {
                    if (i == col) {
                        currentStep+=1;
                    } else {
                        switchToGameOver();
                    }
                }
            }
            if (currentStep > currentLevel) {
                nextLevel();
            } else {
                buffer.drawText(stringBuffer.start()->put(currentStep)->get(),44,52,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
            }
        }


    }

    void loopGameOver() {
        buffer.drawText(stringBuffer.start()->load(PSTR("game over"))->get(),16,28,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("score: "))->put(currentLevel)->get(),24,38,buffer.rgb(192,192,192), &virtualDJ_5ptFontInfo);
        if (progressCounter < 50) {
            progressCounter++;
        } else {
            drawButtons();
            if (ScreenButtonState::wasAnyButtonReleased()) {
                switchToMainMenu();
            }
        }
    }

    void loop() {
        ScreenButtonState::updateButtonState(display.getButtons());

        static unsigned long throttle = millis();
        static unsigned long t = millis();
        unsigned long t2 = millis();

        //buffer.drawText(stringBuffer.start()->put(t2-t)->put("ms")->get(),2,2,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        switch (mode) {
        case DSMODE_MAINMENU: loopMainMenu(); break;
        case DSMODE_PLAYING: loopPlaying(); break;
        case DSMODE_GAMEOVER: loopGameOver(); break;
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
    vx -= vx>>2;
    vy -= vy>>2;
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
    const bool on = isEnabled && ScreenButtonState::isButtonOn(btn);
    const int width = on ? 8 : 6;
    const int height = on ? 24 : 26;
    const int xOffset = isEnabled ? 0 : ((x > 48) * 2 - 1) * 3;

    duinoSays->buffer.drawRect(x - width / 2 + xOffset,y - height / 2,width,height)->filledRect(on ? pressedColor : normalColor);
    if (on && ScreenButtonState::isButtonActivated(btn)) {
        isPressed = true;
        duinoSays->particleSystem.spawn(btn, x,y,x<48?4000:-4000,0, 8);
    } else {
        isPressed = false;
    }
}

DuinoSays duinoSays;


void setup() {
    duinoSays.setup();
}

void loop(void) {
    duinoSays.loop();
}
