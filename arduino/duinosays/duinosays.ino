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

enum DuinoSaysGameMode {
    DSMODE_MAINMENU,
    DSMODE_PLAYING,
    DSMODE_GAMEOVER,
    DSMODE_SHOWSCORES,
    DSMODE_SHOWHELP,
    DSMODE_SHOWABOUT
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
    uint8_t normalColor[3];
    uint16_t pressedColor;
    bool isEnabled;
    bool isPressed;
public:
    Button(ScreenButtonId btn, uint8_t x, uint8_t y, uint8_t normalColorR, uint8_t normalColorG, uint8_t normalColorB, uint16_t pressedColor):
        x(x), y(y), btn(btn), normalColor{normalColorR, normalColorG, normalColorB}, pressedColor(pressedColor), isEnabled(true) {};
    void setEnabled(bool on) {
        isEnabled = on;
    }
    uint8_t* getNormalColor() {
        return normalColor;
    }
    bool getIsPressed() {
        return isPressed;
    }
    void draw(DuinoSays *duinoSays);
};

class Scores {
private:
    uint16_t score[3];
    char tags[3][3];
    uint8_t trackCount;
public:
    Scores():trackCount(0) {

    }

    char* getTag(uint8_t n) {
        if (n < trackCount) return tags[n];
        return 0;
    }
    uint16_t getScore(uint8_t n) {
        if (n < trackCount) return score[n];
        return 0;
    }
    bool isHighEnough(uint8_t points) {
        return trackCount < 3 || points > score[2];
    }
    void putScore(uint8_t points, char* tag) {
        for (int i=0;i<3;i+=1) {
            if (i >= trackCount || score[i] < points) {
                uint16_t tmpPoints = score[i];
                char tmp[3] = {tags[i][0],tags[i][1],tags[i][2]};
                score[i] = points;
                tags[i][0] = tag[0];
                tags[i][1] = tag[1];
                tags[i][2] = tag[2];
                if (trackCount == i) {
                    trackCount += 1;
                }
                // carry over
                points = tmpPoints;
                tag[0] = tmp[0];
                tag[1] = tmp[1];
                tag[2] = tmp[2];
            }
        }
    }

};

class GameOverScreen {
public:
    void loop(DuinoSays* ds);
};


class DuinoSays {
private:
    uint16_t seed;
    uint16_t currentLevel;
    uint16_t currentStep;
    bool isShowingColors;
    DuinoSaysGameMode mode;
    TinyScreen display;
    GameOverScreen gameOverScreen;
public:
    uint16_t progressCounter;
    uint16_t currentScore;
    Scores scores;
    RenderBuffer<uint16_t,40> buffer;
    ParticleSystem<28> particleSystem;
    Button btnTopLeft;
    Button btnTopRight;
    Button btnBottomLeft;
    Button btnBottomRight;

public:
    DuinoSays():
        mode(DSMODE_MAINMENU),
        display(0),
        btnTopLeft(SCREENBUTTON_TOPLEFT, 2, 16, 192,0,0, buffer.rgb(255,0,0)),
        btnTopRight(SCREENBUTTON_TOPRIGHT, 94, 16, 0,192,0, buffer.rgb(0,255,0)),
        btnBottomLeft(SCREENBUTTON_BOTTOMLEFT, 2, 48, 192,192,0, buffer.rgb(255,255,0)),
        btnBottomRight(SCREENBUTTON_BOTTOMRIGHT, 94, 48, 0,0,192, buffer.rgb(64,64,255))
    {

    }

    Button* getButton(uint8_t n) {
        switch (n%4) {
        case SCREENBUTTON_TOPLEFT: return &btnTopLeft;
        case SCREENBUTTON_TOPRIGHT: return &btnTopRight;
        case SCREENBUTTON_BOTTOMLEFT: return &btnBottomLeft;
        case SCREENBUTTON_BOTTOMRIGHT: return &btnBottomRight;
        }
        return 0;
    }

    void setup() {
        Wire.begin();
        #if defined(ARDUINO_ARCH_SAMD)
        display.begin(TinyScreenPlus);
        #else
        display.begin();
        #endif
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

    void loopHelp() {
        buffer.drawText(stringBuffer.start()->load(PSTR("got it"))->get(),60,45,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("duino says a"))->get(),1,1,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("sequence of"))->get(),1,9,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("colors."))->get(),1,17,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("push the buttons"))->get(),1,25,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("and repeat it."))->get(),1,33,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);

        btnBottomRight.draw(this);
        if (btnBottomRight.getIsPressed()) {
            switchToMainMenu();
        }
    }

    void drawDuinoSaysTitle(int8_t yOffset) {
        progressCounter+=1;
        uint8_t step = progressCounter % 64;
        for (int i=-8;i<=8;i+=1) {

            //buffer.drawRect(i*12 + 48 + (step),27,10,1)->filledRect(buffer.rgb(255 - abs(step*8-255),0,0));
            int8_t x = (i*16 - (step))%64+68;
            int16_t tone = 255 - abs(x*8-320);
            if (tone > 255) tone = 255;
            else if (tone <= 0) continue;
            Button *btn = getButton(i&3);
            uint8_t *rgb = btn->getNormalColor();
            uint16_t color = buffer.rgb((uint8_t)(tone * rgb[0] >> 8),(uint8_t)(tone * rgb[1] >> 8),(uint8_t)(tone * rgb[2] >> 8));

            buffer.drawRect(x,35 + yOffset,8,1)->filledRect(color);
            buffer.drawRect(80-x,27 + yOffset,8,1)->filledRect(color);
        }

        buffer.drawText(stringBuffer.start()->load(PSTR("duino says ..."))->get(),16,28 + yOffset,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
    }

    void loopMainMenu() {
        drawDuinoSaysTitle(0);
        //buffer.drawText(stringBuffer.start()->load(PSTR("push button"))->get(),16,45,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("start"))->get(),60,45,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("scores"))->get(),7,45,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);

        buffer.drawText(stringBuffer.start()->load(PSTR("help"))->get(),7,13,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("about"))->get(),60,13,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        drawButtons();
        if (progressCounter++%21 == 0) {
            //spawnQuadParticles((progressCounter%4), 48, 32, 1000, 40);
        }

        if (btnBottomRight.getIsPressed())
        {
            switchToPlaying();
        }
        if (btnTopLeft.getIsPressed())
        {
            switchToHelp();
        }
        if (btnTopRight.getIsPressed())
        {
            switchToAbout();
        }
        if (btnBottomLeft.getIsPressed())
        {
            switchToScores();
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

    void switchToHelp() {
        mode = DSMODE_SHOWHELP;
    }
    void switchToAbout() {
        mode = DSMODE_SHOWABOUT;
    }
    void switchToScores() {
        mode = DSMODE_SHOWSCORES;
    }

    void switchToPlaying() {
        mode = DSMODE_PLAYING;
        currentLevel = 0;
        seed = millis();
        nextLevel();
        currentScore = 0;
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

    void spawnQuadParticles(uint8_t col, uint8_t x, uint8_t y, int16_t speed, uint8_t age) {
        particleSystem.spawn(col, x,y, speed, speed, age);
        particleSystem.spawn(col, x,y, speed,-speed, age);
        particleSystem.spawn(col, x,y,-speed,-speed, age);
        particleSystem.spawn(col, x,y,-speed, speed, age);
    }

    void loopPlaying() {
        drawButtons();
        if (isShowingColors) {
            const uint8_t progressCounterMaxBit = 2;
            if (currentStep == 0) {
                buffer.drawText(stringBuffer.start()->load(PSTR("duino says ..."))->get(),16,28,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
            }
            if (currentStep <= currentLevel && currentStep > 0) {
                const uint8_t w = 4 + progressCounter, h = w;
                uint8_t col = getColor(currentStep);
                Button* button = getButton(col);
                uint8_t* btnColor = button->getNormalColor();
                uint16_t rectColor = buffer.rgb(
                                                btnColor[0] * progressCounter >> progressCounterMaxBit,
                                                btnColor[1] * progressCounter >> progressCounterMaxBit,
                                                btnColor[2] * progressCounter >> progressCounterMaxBit);
                buffer.drawRect(48-w/2,32-h/2,w,h)->filledRect(rectColor);
                buffer.drawText(stringBuffer.start()->putDec(currentStep)->put(":")->putDec(currentLevel)->get(),42,52,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
                if (progressCounter == 0) {
                    spawnQuadParticles(4|col, 48,32,800,24);
                }
            }
            if (progressCounter++ > (1<<progressCounterMaxBit)) {
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
                        currentScore += 1;
                    } else {
                        switchToGameOver();
                    }
                }
            }
            if (currentStep > currentLevel) {
                nextLevel();
            } else {
                buffer.drawText(stringBuffer.start()->putDec(currentStep)->put(":")->putDec(currentLevel)->get(),42,52,buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
            }
        }


    }


    void loopAbout() {
        drawDuinoSaysTitle(-24);
        buffer.drawText(stringBuffer.start()->load(PSTR("by eike decker"))->get(),10,15,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("tinyduinogames.de"))->get(),2,25,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawText(stringBuffer.start()->load(PSTR("cool"))->get(),67,45,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        btnBottomRight.draw(this);
        if (btnBottomRight.getIsPressed()) {
            switchToMainMenu();
        }
    }

    void loopScores() {
        buffer.drawText(stringBuffer.start()->load(PSTR("scores"))->get(),30,2,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        buffer.drawRect(8,10,80,1)->filledRect(buffer.rgb(255,255,255));

        for (int i=0;i<3;i+=1) {
            char *tag = scores.getTag(i);
            if (tag == 0) break;
            uint16_t score = scores.getScore(i);
            buffer.drawText(stringBuffer.start()->putDec(i)->put(')')->get(),10,15 + i * 8,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
            buffer.drawText(stringBuffer.start()->put(tag)->get(),38,15 + i * 8,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
            buffer.drawText(stringBuffer.start()->putDec(score)->get(),81 - score / 10 * 5,15 + i * 8,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        }

        buffer.drawText(stringBuffer.start()->load(PSTR("beat it"))->get(),53,45,buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);

        btnBottomRight.draw(this);
        if (btnBottomRight.getIsPressed()) {
            switchToMainMenu();
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
        case DSMODE_GAMEOVER: gameOverScreen.loop(this); break;
        case DSMODE_SHOWHELP: loopHelp(); break;
        case DSMODE_SHOWABOUT: loopAbout(); break;
        case DSMODE_SHOWSCORES: loopScores(); break;
        }

        particleSystem.draw(this);

        t = t2;
        buffer.flush(display);
        stringBuffer.reset();

        while (millis() - throttle < 50) continue;
        throttle = millis();
    }
};


void GameOverScreen::loop(DuinoSays* ds) {
    ds->buffer.drawText(stringBuffer.start()->load(PSTR("game over"))->get(),20,16,ds->buffer.rgb(255,128,32), &virtualDJ_5ptFontInfo);
    ds->buffer.drawText(stringBuffer.start()->load(PSTR("score: "))->putDec(ds->currentScore)->get(),
                        28 - (ds->currentScore / 10) * 5,40,ds->buffer.rgb(192,192,192), &virtualDJ_5ptFontInfo);
    if (ds->progressCounter < 50) {
        ds->progressCounter++;
    } else {
        if (ds->scores.isHighEnough(ds->currentScore)) {
            static uint8_t tag[3] = {0, 0, 0};
            static uint8_t tagPos = 0;
            const char* alpha = " abcdefghijklmnopqrstuvwxyz.-!?0123456789";
            uint8_t currentPos = tag[tagPos];
            char prevIdx = (currentPos + 41 - 1) % (41);
            char nextIdx = (currentPos + 41 + 1) % (41);

            const uint8_t grey = 128;

            ds->buffer.drawText(stringBuffer.start()->put(alpha[prevIdx])->get(),7,13,ds->buffer.rgb(grey,grey,grey), &virtualDJ_5ptFontInfo);
            ds->buffer.drawText(stringBuffer.start()->put(alpha[nextIdx])->get(),7,45,ds->buffer.rgb(grey,grey,grey), &virtualDJ_5ptFontInfo);
            ds->buffer.drawText(stringBuffer.start()->load(PSTR(">"))->get(),86,13,ds->buffer.rgb(grey,grey,grey), &virtualDJ_5ptFontInfo);
            ds->buffer.drawText(stringBuffer.start()->load(PSTR("ok"))->get(),78,45,ds->buffer.rgb(grey,grey,grey), &virtualDJ_5ptFontInfo);

            ds->buffer.drawText(stringBuffer.start()->put(alpha[tag[0]])->get(),39,28,ds->buffer.rgb(192,192,192), &virtualDJ_5ptFontInfo);
            ds->buffer.drawText(stringBuffer.start()->put(alpha[tag[1]])->get(),46,28,ds->buffer.rgb(192,192,192), &virtualDJ_5ptFontInfo);
            ds->buffer.drawText(stringBuffer.start()->put(alpha[tag[2]])->get(),53,28,ds->buffer.rgb(192,192,192), &virtualDJ_5ptFontInfo);

            ds->buffer.drawRect(39,35,5,tagPos == 0 ? 2 : 1)->filledRect(ds->buffer.rgb(255,255,255));
            ds->buffer.drawRect(46,35,5,tagPos == 1 ? 2 : 1)->filledRect(ds->buffer.rgb(255,255,255));
            ds->buffer.drawRect(53,35,5,tagPos == 2 ? 2 : 1)->filledRect(ds->buffer.rgb(255,255,255));


            ds->drawButtons();
            if (ds->btnTopLeft.getIsPressed()) {
                tag[tagPos] = prevIdx;
            }
            if (ds->btnBottomLeft.getIsPressed()) {
                tag[tagPos] = nextIdx;
            }
            if (ds->btnTopRight.getIsPressed()) {
                tagPos = (tagPos + 1) % 3;
            }

            if (ds->btnBottomRight.getIsPressed()) {
                char tagName[3] = {alpha[tag[0]],alpha[tag[1]],alpha[tag[2]]};
                ds->scores.putScore(ds->currentScore, tagName);
                ds->switchToScores();
            }

        } else {
            ds->buffer.drawText(stringBuffer.start()->load(PSTR("OK"))->get(),85,45,ds->buffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
            ds->btnBottomRight.draw(ds);


            if (ds->btnBottomRight.getIsPressed()) {
                ds->switchToMainMenu();
            }
        }

    }

}

void Particle::draw(DuinoSays* duinoSays) {
    const uint8_t rel = 255 - frameAge * 255 / maxAge;
    const bool isButtonParticle = type < 4;
    const int height = isButtonParticle ? 24 : 8 * rel >> 8;
    frameAge+=1;

    const int width = isButtonParticle ? 16 * rel >> 8 : height;

    uint8_t r = 255, g = 0, b = 255;
    switch (type & 3) {
        case ParticleType::topLeftButton: r = 255, g = 64, b = 32; break;
        case ParticleType::bottomLeftButton: r = 255, g = 255, b = 64; break;
        case ParticleType::topRightButton: r = 32, g = 255, b = 32; break;
        case ParticleType::bottomRightButton: r = 64, g = 64, b = 255; break;
    }

    duinoSays->buffer.drawRect((x>>8) - width / 2,(y>>8) - height / 2,width,height)->filledRect(duinoSays->buffer.rgb(rel * r >> 8,rel * g >> 8,rel * b >> 8));
    x+=vx;
    y+=vy;
    if ( isButtonParticle) {
        vx -= vx>>2;
        vy -= vy>>2;
    } else {
        vx -= (vy>>2) + (vx >> 5);
        vy -= (-vx>>2) + (vy >> 5);
    }

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
    const uint16_t normalColor565 = duinoSays->buffer.rgb(normalColor[0], normalColor[1], normalColor[2]);

    duinoSays->buffer.drawRect(x - width / 2 + xOffset,y - height / 2,width,height)->filledRect(on ? pressedColor : normalColor565);
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
