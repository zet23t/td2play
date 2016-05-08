#include "game_main.h"
#include "lib_fixedmath.h"
#include "lib_input.h"
#include "asset_tilemap.h"
#include "image_data.h"

namespace Game{
    struct LevelElement {
        TileMap::Scene<uint16_t> *element;
        unsigned int offset;
    };
    struct Player {
        Fixed2D4 pos;
        Fixed2D4 vel;
    };
    Texture<uint16_t> tiles;
    Texture<uint16_t> imageTiles;
    unsigned int levelX, levelTime;
    Player player;
    bool isRunning, isGameOver;

    TileMap::SceneRenderer<uint16_t,RENDER_COMMAND_COUNT> renderer;
    TileMap::Scene<uint16_t> skymap;
    int elementCount;
    TileMap::Scene<uint16_t> *elements;



    LevelElement levelElements[3];

    static void restart() {
        isRunning = false;
        isGameOver = false;
        levelX = 0;
        levelElements[0].element = &elements[0];
        levelElements[1].element = &elements[0];
        levelElements[2].element = &elements[0];
        levelElements[0].offset = 0;
        levelElements[1].offset = levelElements[0].element->calcWidth();
        levelElements[2].offset = levelElements[1].element->calcWidth() + levelElements[1].offset;
        player.pos.x.setIntegerPart(20);
        player.pos.y.setIntegerPart(10);
    }

    void setup() {
        tiles = Texture<uint16_t>(ImageAsset::ztiles_foreground);
        imageTiles = Texture<uint16_t>(ImageAsset::ztiles_sky);
        skymap = TilemapAsset::skymap();
        static TileMap::Scene<uint16_t> elList[] = {
            TilemapAsset::el_01(),
            TilemapAsset::el_02(),
            TilemapAsset::el_03(),
            TilemapAsset::el_04(),
            TilemapAsset::el_05(),
        };
        elements = elList;
        elementCount = sizeof(elList) / sizeof(TileMap::Scene<uint16_t>);

        buffer.setClearBackground(false,buffer.rgb(108,120,200));
        restart();
    }

    void loop() {
        if (isRunning)
            levelX += 1;
        levelTime += 1;

        if (player.pos.x.getIntegerPart() < -5 || player.pos.y.getIntegerPart() > 90) {
            isGameOver = true;
            isRunning = false;
        } else {
            player.pos += player.vel;
            player.vel.y *= FixedNumber16<4>(0,12);
            if (player.vel.y.absolute().getRaw() < 4) player.vel.y = 0;
        }

        Fixed2D4 nextBlocked = player.pos + Fixed2D4(FixedNumber16<4>(-1,0), FixedNumber16<4>(0,0));
        Fixed2D4 nextFallBlocked = player.pos + Fixed2D4(FixedNumber16<4>(-1,0), FixedNumber16<4>(2,0));
        Fixed2D4 nextFall = player.pos + Fixed2D4(FixedNumber16<4>(0,0), FixedNumber16<4>(2,0));
        bool isBlocked = false;
        bool isFalling = true;
        bool isGrounded = false;
        renderer.update(buffer, skymap, 48, 80,0,1,0);

        for (int i=0;i<3;i+=1) {
            int localX = levelX - levelElements[i].offset;
            int fallx = nextFall.x.getIntegerPart()+localX;
            int fally = nextFall.y.getIntegerPart()-32+20-1;
            int blockedx = nextBlocked.x.getIntegerPart()+localX;
            int blockedy = nextBlocked.y.getIntegerPart()-32+20-1;
            if (!levelElements[i].element->isRectFree(fallx-1,fally,fallx,fally+5)) {
                isGrounded = true;
            }
            if (!levelElements[i].element->isRectFree(fallx,fally,fallx+2,fally+4)) {
                isFalling = false;
            }
            if (!levelElements[i].element->isRectFree(blockedx,blockedy,blockedx+2,blockedy+4)) {
                isBlocked = true;
            }

            renderer.update(buffer, *levelElements[i].element, 48+levelX-levelElements[i].offset,20,0,1,0);
        }
        if (!isGameOver) {
            if (isFalling) player.pos = nextFall;
            else if (isBlocked) player.pos = !isGrounded ? nextFallBlocked : nextBlocked;

            if (isGrounded) {
                static int groundTime = 0;
                if ((Joystick::getButton(0) || Joystick::getButton(1)) && groundTime > 5) {
                    player.vel.y -= FixedNumber16<4>(6,0);
                    isRunning = true;
                    groundTime = 0;
                }
                groundTime += 1;

            }
        }

        if (levelX > levelElements[1].offset) {
            levelElements[0] = levelElements[1];
            levelElements[1] = levelElements[2];
            levelElements[2].offset = levelElements[1].offset + levelElements[1].element->calcWidth();
            levelElements[2].element = &elements[Math::randInt()%elementCount];
            //printf("%d %d %d\n",levelElements[0].offset,levelElements[1].offset,levelElements[2].offset);
        }


        //buffer.drawRect(player.pos.x.getIntegerPart(),player.pos.y.getIntegerPart(),2,4)->filledRect(buffer.rgb(200,0,0));
        int u = 106;
        if (isRunning) {
            u = levelX/4%2 ? 111 : 116;
        }
        if (!isGrounded || isBlocked) u = 121;

        buffer.drawRect(player.pos.x.getIntegerPart()-2,player.pos.y.getIntegerPart()-3,4,8)->sprite(&tiles,u,0);

        static int gameoverTimer = 0;
        if (isGameOver) {
            //buffer.drawRect(player.pos.x.getIntegerPart()-2,player.pos.y.getIntegerPart()-3,4,8)->sprite(&tiles,u,0);
            buffer.drawRect(0,16,96,16)->sprite(&imageTiles,0,14);
            buffer.drawRect(0,33,96,14)->sprite(&imageTiles,0,0);
            gameoverTimer+=1;
            if (gameoverTimer > 20 && (Joystick::getButton(0) || Joystick::getButton(1))) {
                gameoverTimer = 0;
                restart();
            }
        } else if (!isRunning) {
            buffer.drawRect(0,16,96,16)->sprite(&imageTiles,0,30);
            if (levelTime / 16 % 2)
                buffer.drawRect(player.pos.x.getIntegerPart()+2,player.pos.y.getIntegerPart()-15,49,15)->sprite(&imageTiles,0,46);
        }

    }
}
