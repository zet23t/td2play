#ifndef __GAME_H__
#define __GAME_H__

#include "lib_FixedMath.h"
#include "lib_RenderBuffer.h"
#include <algorithm>
#include "sprites.h"

extern RenderBuffer<uint16_t,128> renderBuffer;

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

template<uint8_t width, uint8_t height, uint8_t tileSizeBits>
class TileMap {
private:
    uint16_t *tiles;
    int16_t offsetX, offsetY;
    uint8_t paralax;
public:
    TileMap(uint16_t offsetX, uint16_t offsetY, uint8_t paralax, uint16_t *tiles)
        : offsetX(offsetX), offsetY(offsetY), paralax(paralax), tiles(tiles)
    {

    }
    void update(const Camera &camera) {
        const int16_t centerX = camera.position.x.getIntegerPart();
        const int16_t centerY = camera.position.y.getIntegerPart();
        int16_t topLeftX = (centerX & ~((1<<tileSizeBits)-1)) - (RenderBufferConst::screenWidth >> 1);
        int16_t topLeftY = (centerY  & ~((1<<tileSizeBits)-1))- (RenderBufferConst::screenHeight >> 1);
        const int16_t minX = topLeftX;
        const int16_t maxX = topLeftX + RenderBufferConst::screenWidth + (1 << tileSizeBits);
        const int16_t minY = topLeftY;
        const int16_t maxY = topLeftY + RenderBufferConst::screenHeight + (1 << tileSizeBits);

        for (int16_t y = minY; y < maxY; y += 1 << tileSizeBits) {
            for (int16_t x = minX; x < maxX; x += 1 << tileSizeBits) {
                if (y < 0 || x < 0 || x >= width << tileSizeBits || y >= height << tileSizeBits) continue;
                const uint8_t tileX = x >> tileSizeBits;
                const uint8_t tileY = y >> tileSizeBits;
                const uint16_t tileIndex = tiles[tileX + tileY * width];

                renderBuffer.drawRect(x + (RenderBufferConst::screenWidth>>1) - centerX,
                                      y + (RenderBufferConst::screenHeight>>1) - centerY,8,8)
                                      ->sprite(&texture::beastlands,
                                               (tileIndex & 0xff) << tileSizeBits,
                                               (tileIndex >> 8) << tileSizeBits);
                                      //->filledRect(renderBuffer.rgb(255&(x+64),255&(y+64),0));
            }
        }

    }
};

class Pawn : public Body {
private:
public:

    void update() {
    }
};

static uint16_t levelMap[] = {
    0,1,2,0, 0,0,0,0,
    1,3,4,5, 0,0,0,0,
    0x108,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,

    0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,
    0,0,0,0, 0,0,0,0,
};

class LevelMapScreen : public Screen {
    TileMap<8,8,3> tileMap;
    Camera camera;
public:
    LevelMapScreen(): tileMap(0,0,0,levelMap), camera() {
    }
    void update() {
        camera.position += Fixed2D4(0,8,0,8);
        //printf("%s\n",camera.position.x.toString());
        tileMap.update(camera);
    }
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
    Game():mainMenuScreen(), currentScreen(&levelMapScreen) {
    }
    void update() {
        currentScreen->update();
    }
};

#endif // __GAME_H__
