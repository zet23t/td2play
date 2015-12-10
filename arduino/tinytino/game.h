#ifndef __GAME_H__
#define __GAME_H__

#include "lib_FixedMath.h"
#include "lib_RenderBuffer.h"

extern RenderBuffer<uint16_t,40> renderBuffer;

class Screen {
public:
    virtual void update() {}
};

class MainMenuScreen : public Screen {
public:
    MainMenuScreen() {

    }
    void update() {
        FixedNumber<4> fixA(42,15);
        FixedNumber<4> fixB(17,3);
        renderBuffer.drawText((fixA + fixB).toString(),16,8,renderBuffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        renderBuffer.drawRect(39,35,5,5)->filledRect(renderBuffer.rgb(255,255,255));
    }
};

class Game {
private:
    MainMenuScreen mainMenuScreen;

    Screen* currentScreen;
public:
    Game():mainMenuScreen(), currentScreen(&mainMenuScreen) {
    }
    void update() {
        currentScreen->update();
    }
};

#endif // __GAME_H__
