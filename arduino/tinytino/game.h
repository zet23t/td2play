#ifndef __GAME_H__
#define __GAME_H__

#include "lib_FixedMath.h"
#include "lib_RenderBuffer.h"
#include "lib_tilemap.h"
#include "level_test2.h"
#include "sprites.h"

#include "game_types.h"

#include "screen.h"
#include "physics.h"

#include "LevelMapScreen.h"



class MainMenuScreen : public Screen {
public:
    MainMenuScreen() {

    }
    void init() {
    }
    void update() {
        FixedNumber16<4> fixA(42,15);
        FixedNumber16<4> fixB(17,1);
        renderBuffer.drawText((fixA + fixB).toString(),16,8,renderBuffer.rgb(255,255,255), &virtualDJ_5ptFontInfo);
        renderBuffer.drawRect(39,35,5,5)->filledRect(renderBuffer.rgb(255,255,255));
    }
};

class Game {
private:
    MainMenuScreen mainMenuScreen;
    LevelMapScreen levelMapScreen;
    Screen* currentScreen;
public:
    Game():mainMenuScreen(), levelMapScreen(), currentScreen(0) {

    }
    void update() {
        if (currentScreen == 0) {
            TileMap::SceneBgFg<uint16_t> scene = Level_test2::getScene();
            levelMapScreen.load(scene);
            currentScreen = &levelMapScreen;
            currentScreen->init();
        }
        currentScreen->update();
    }
};

#endif // __GAME_H__
