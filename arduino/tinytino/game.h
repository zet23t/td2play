#ifndef __GAME_H__
#define __GAME_H__

#include "lib_FixedMath.h"
#include "lib_RenderBuffer.h"
#include "lib_tilemap.h"
#include "level_test2.h"
#include "sprites.h"

#include "game_types.h"

#define MAX_BODIES 64

class Screen {
public:
    virtual void init() {}
    virtual void update() {}
};

class World;
class Camera;

class Body {
public:
    World *world;
    Fixed2D4 position;
    Fixed2D4 velocity;
    int id;
    Body():world(0),position(0,0), velocity(0,0), id(0) {}
    void updateStep(Camera &camera);
};


class Camera : public Body {
public:
};

class World {
public:
    TileMap::SceneBgFg<uint16_t> *scene;
    Body bodyList[MAX_BODIES];
    uint8_t bodyCount;

    World():scene(0),bodyCount(0){}
    void init();
    void addBody(Body &body);
    void updateStep(Camera &camera);
    bool isFree(const Fixed2D4& pos) const;
    Fixed2D4 moveOut(const Fixed2D4& pos) const;
};





class Pawn : public Body {
private:
public:

    void update() {
    }
};


class LevelMapScreen : public Screen {
    TileMap::SceneBgFg<uint16_t> scene;
    TileMap::SceneBgFgRenderer<uint16_t, 200> renderer;
    World world;
    Camera camera;
public:
    LevelMapScreen() {

    }
    LevelMapScreen(TileMap::SceneBgFg<uint16_t> &scene): scene(scene), world(), camera() {
    }
    void load(TileMap::SceneBgFg<uint16_t>& scene) {
        this->scene = scene;
    }
    void init();
    void update();

};

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
