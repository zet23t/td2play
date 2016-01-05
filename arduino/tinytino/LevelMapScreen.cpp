#include "game.h"
#include "tilemap.h"
#include "lib_input.h"
#include "lib_tilemap.h"
#include "lib_math.h"
#include "texturemap.h"

void LevelMapScreen::init() {

    world.init();
    for (int i=0;i<20;i+=1) {
        Body player;
        player.id = 1;
        player.spriteW = 2;
        player.spriteH = 2;
        player.position.setXY(100,100);
        player.velocity.setXY((Math::randInt()&0xf) - 7, (Math::randInt()&0xf)-7);

        world.addBody(player);
    }
    world.scene = &scene;
    camera.position = Fixed2D4(100,104);
    renderBuffer.setClearBackground(true,renderBuffer.rgb(80,60,70));
}

void LevelMapScreen::update() {
    static int frame = 0;
    frame += 1;

    if (frame % 100 == 0) {
        for (int i=0;i<5;i+=1) {
            Body* player = world.getBody(i);
            player->sprite = &TextureData::ztiles_foreground;
            player->spriteW = 8;
            player->spriteH = 8;
            player->spriteX = 104;
            player->spriteY = 0;
            player->position.setXY(110,100);
            player->velocity.randomCircle(FixedNumber16<4>(3,0));
            //player->velocity.setXY((Math::randInt()&7) - 3, (Math::randInt()&7)-3);
        }
    }

    Fixed2D4 offset =
        (ScreenButtonState::isButtonOn(SCREENBUTTON_BOTTOMRIGHT) ? Fixed2D4(0,8,0,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_BOTTOMLEFT) ? Fixed2D4(-1,8,0,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_TOPRIGHT) ? Fixed2D4(0,8,-1,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_TOPLEFT) ? Fixed2D4(-1,8,-1,8) : Fixed2D4(0,0));

    camera.position += offset + Joystick::getJoystick()*8;
    renderer.update(renderBuffer, scene, camera.position.x.getIntegerPart(), camera.position.y.getIntegerPart());
    world.updateStep(camera);

    Math::Vector2D16 pos(camera.position.x.getIntegerPart(), camera.position.y.getIntegerPart());
    Math::Vector2D16 result = scene.moveOut(pos);
    uint8_t index;
    bool isFree = true; //scene.isPixelFree(pos.x,pos.y,index);
    renderBuffer.drawRect(result.x - pos.x + 48, result.y - pos.y + 32,1,1)
                                    ->filledRect(renderBuffer.rgb(isFree ? 255 : 0,90,30));
    //tileMap->update(camera);
}
