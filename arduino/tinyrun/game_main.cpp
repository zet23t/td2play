#include "game_main.h"
#include "lib_fixedmath.h"
#include "lib_input.h"
#include "asset_tilemap.h"
#include "image_data.h"
#include "font_asset.h"

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
            TilemapAsset::el_06(),
            TilemapAsset::el_07(),
            TilemapAsset::el_08(),
            TilemapAsset::el_09(),
            TilemapAsset::el_10(),
            TilemapAsset::el_11(),
        };
        elements = elList;
        elementCount = sizeof(elList) / sizeof(TileMap::Scene<uint16_t>);

        buffer.setClearBackground(true,RGB565(140,162,189));
        restart();
    }

    static bool isButtonPressed() {
        return Joystick::getButton(0) || Joystick::getButton(1) || ScreenButtonState::isAnyButtonOn();
    }

    struct Particle {
        Fixed2D4 pos, vel;
    };

    void drawParticles(int infernoX) {
        const int maxParticles = 12;
        static Particle particles[maxParticles];
        for (int i=0;i<maxParticles;i+=1) {
            Particle *p = &particles[i];
            if (p->pos.y <= 0 || p->pos.y > FixedNumber16<4>(64,0)) {
                p->pos.x = FixedNumber16<4>(-(Math::randInt()%8),0) ;
                p->pos.y = FixedNumber16<4>(Math::randInt()%32 + 24,0);
                p->vel.x = FixedNumber16<4>(Math::randInt()%3+1,0);
                p->vel.y = FixedNumber16<4>(-Math::randInt()%5-10,0);
            }
            p->pos += p->vel;
            p->vel.y += FixedNumber16<4>(1,0);
            p->vel.scale(FixedNumber16<4>(0,12));
            int w = 8, h = 8, u = 51, v = 48;
            switch (i%3) {
            case 0: u = 50; v = 49; w=3,h=4; break;
            case 1: u = 66; v = 48; w=5;h=5; break;
            }
            buffer.drawRect(p->pos.x.getIntegerPart()- infernoX+75,p->pos.y.getIntegerPart(),w,h)->sprite(&imageTiles,u,v)->setDepth(41);
        }
    }

    void loop() {

        Math::randInt();
        Fixed2D4 shake = Fixed2D4();

        if (isRunning) {
            levelX += 1;
            int shakeStrenght = 0;
            int playerX = player.pos.x.getIntegerPart();
            if (playerX < 0) playerX = 0;
            shakeStrenght = (20 - playerX) / 4;

            if (shakeStrenght > 0) {

                shake.x = FixedNumber16<4>(Math::randInt()%shakeStrenght-shakeStrenght/2,0);
                shake.y = FixedNumber16<4>(Math::randInt()%shakeStrenght-shakeStrenght/2,0);
            }
        }
        levelTime += 1;

        if (player.pos.x.getIntegerPart() < -5 || player.pos.y.getIntegerPart() > 75) {
            isGameOver = true;
            isRunning = false;
        } else {
            player.pos += player.vel;
            //player.vel.y *= FixedNumber16<4>(0,13);
            if (player.vel.y.getRaw() < 0) player.vel.y += FixedNumber16<4>(0,6);
            if (player.vel.y.absolute().getRaw() < 4) player.vel.y = 0;
        }

        Fixed2D4 nextBlocked = player.pos + Fixed2D4(FixedNumber16<4>(-1,0), FixedNumber16<4>(0,0));
        Fixed2D4 nextFallBlocked = player.pos + Fixed2D4(FixedNumber16<4>(-1,0), FixedNumber16<4>(2,0));
        Fixed2D4 nextFall = player.pos + Fixed2D4(FixedNumber16<4>(0,0), FixedNumber16<4>(2,0));
        bool isBlocked = false;
        bool isFalling = true;
        bool isGrounded = false;

        int shakeX = shake.x.getIntegerPart();
        int shakeY = shake.y.getIntegerPart();

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
            int x = 48+levelX-levelElements[i].offset-shakeX;
            int y = 20-shakeY;
            renderer.update(buffer, *levelElements[i].element, x,y,1,1,20);
            renderer.update(buffer, *levelElements[i].element, x,y,0,1,10);
        }
        if (!isGameOver) {
            if (isFalling) player.pos = nextFall;
            else if (isBlocked) player.pos = !isGrounded ? nextFallBlocked : nextBlocked;

            if (isGrounded) {
                static int groundTime = 0;
                if ((isButtonPressed()) && groundTime > 5) {
                    player.vel.y -= FixedNumber16<4>(5,0);
                    isRunning = true;
                    groundTime = 0;
                }
                groundTime += 1;

            } else if (player.vel.y.getRaw() < 0 && !(isButtonPressed())){
                player.vel.y = 0;//FixedNumber16<4>(0,1);
            }
        }

        if (levelX > levelElements[1].offset) {
            levelElements[0] = levelElements[1];
            levelElements[1] = levelElements[2];
            levelElements[2].offset = levelElements[1].offset + levelElements[1].element->calcWidth();
            levelElements[2].element = &elements[Math::randInt()%(elementCount-1)+1];
            //printf("%d %d %d\n",levelElements[0].offset,levelElements[1].offset,levelElements[2].offset);
        }


        //buffer.drawRect(player.pos.x.getIntegerPart(),player.pos.y.getIntegerPart(),2,4)->filledRect(buffer.rgb(200,0,0));
        int u = 106;
        if (isRunning) {
            u = levelX/4%2 ? 111 : 116;
        }
        if (!isGrounded || isBlocked) u = 121;

        buffer.drawRect(player.pos.x.getIntegerPart()-2+shakeX,player.pos.y.getIntegerPart()-3+shakeY,4,8)->sprite(&tiles,u,0)->setDepth(45);

        static int gameoverTimer = 0;
        bool restartNext = false;
        if (isGameOver) {
            if (player.pos.x.getIntegerPart() > -140) player.pos.x -= FixedNumber16<4>(4,0);
            //buffer.drawRect(player.pos.x.getIntegerPart()-2,player.pos.y.getIntegerPart()-3,4,8)->sprite(&tiles,u,0);
            buffer.drawRect(0,16,96,16)->sprite(&imageTiles,0,14)->setDepth(100);
            buffer.drawRect(0,33,96,14)->sprite(&imageTiles,0,0)->setDepth(100);
            buffer.drawText(stringBuffer.putDec(levelX / 2).get(),62,36,34,0,FontAsset::digits,100);
            gameoverTimer+=1;
            if (gameoverTimer > 20 && (isButtonPressed())) {
                gameoverTimer = 0;
                restartNext = true;
            }
        } else if (!isRunning) {
            buffer.drawRect(0,16,96,16)->sprite(&imageTiles,0,30)->setDepth(100);
            if (levelTime / 64 % 2)
                buffer.drawRect(15,56,63,5)->sprite(&imageTiles,1,62)->setDepth(100);
            else
                buffer.drawRect(20,56,53,5)->sprite(&imageTiles,8,67)->setDepth(100);
            if (levelTime / 16 % 2)
                buffer.drawRect(player.pos.x.getIntegerPart()+2,player.pos.y.getIntegerPart()-15,49,15)->sprite(&imageTiles,0,46)->setDepth(100);
        } else {
            int offset = 10 - levelX / 5;
            if (offset < 0) offset = 0;
            buffer.drawRect((96-36)/2,-offset,36,10)->sprite(&imageTiles,60,4)->setDepth(100);
            buffer.drawText(stringBuffer.putDec(levelX / 2).get(),0,-offset,96,0,FontAsset::digits,100);
        }

        // rendering front to back using depth buffer to avoid overwriting texture values

        // inferno
        int infernoFactor = (player.pos.x * FixedNumber16<4>(0,12)).getIntegerPart();
        int infernoX = 78 + infernoFactor;
        int infernoY = 83 - levelTime*8 % skymap.calcHeight();
        drawParticles(infernoX);
        infernoX-=24;
        renderer.update(buffer, skymap, infernoX, infernoY,4,1,40);
        renderer.update(buffer, skymap, infernoX, infernoY + skymap.calcHeight(),4,1,40);
        buffer.drawRect(-infernoX-32-16-8,0,104,64)->filledRect(RGB565(0,0,0))->setDepth(40);

        // water
        renderer.update(buffer, skymap, 48 + (levelX%skymap.calcWidth()), 76,3,1,3);
        renderer.update(buffer, skymap, 48 + (levelX%skymap.calcWidth()) - skymap.calcWidth(), 76,3,1,3);

        // clouds
        renderer.update(buffer, skymap, 48 + (levelX/8%skymap.calcWidth()), 90 + player.pos.y.getIntegerPart() / 12,1,1,2);
        renderer.update(buffer, skymap, 48 + (levelX/8%skymap.calcWidth()) - skymap.calcWidth(), 90 + player.pos.y.getIntegerPart() / 12,1,1,2);

        // mountains
        renderer.update(buffer, skymap, 48 + (levelX/12%skymap.calcWidth()), 77,2,1,1);
        renderer.update(buffer, skymap, 48 + (levelX/12%skymap.calcWidth()) - skymap.calcWidth(), 77,2,1,1);

        // sky
        int skyoffset = (infernoFactor < 0 ? 0 : infernoFactor + infernoFactor / 4) - 5;
        renderer.update(buffer, skymap, 48, 80 + player.pos.y.getIntegerPart() / 16 + skyoffset,0,1,0);

        if (restartNext) restart();
    }
}
