#ifndef __GAME_H__
#define __GAME_H__

#include "lib_FixedMath.h"
#include "lib_RenderBuffer.h"
#include <algorithm>
#include "sprites.h"

#include "game_types.h"


class Screen {
public:

    virtual void update() {}
};

class Body {
public:
    Fixed2D4 position;
    Fixed2D4 velocity;
    Body():position(0,0), velocity(0,0) {}
};

class Camera : public Body {
public:
};


class Pawn : public Body {
private:
public:

    void update() {
    }
};


class LevelMapScreen : public Screen {
    TileMap* tileMap;
    Camera camera;
public:
    LevelMapScreen(TileMap *tileMap): tileMap(tileMap), camera() {
    }
    void update();

};

class MainMenuScreen : public Screen {
public:
    MainMenuScreen() {

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
    Game():mainMenuScreen(), levelMapScreen(&map_0), currentScreen(&levelMapScreen) {
    }
    void update() {
        currentScreen->update();
    }
};

#endif // __GAME_H__
