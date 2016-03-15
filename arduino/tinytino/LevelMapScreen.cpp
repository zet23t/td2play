#include "game.h"
#include "lib_tilemap.h"
#include "lib_input.h"
#include "lib_tilemap.h"
#include "lib_math.h"
#include "image_data.h"

void LevelMapScreen::init() {
    static Texture<uint16_t> foreground = Texture<uint16_t>(ImageAsset::ztiles_foreground);
    world.init();
    /*for (int i=0;i<20;i+=1) {
        Body player;
        player.id = 1;
        player.spriteW = 2;
        player.spriteH = 2;
        player.position.setXY(100,100);
        player.velocity.setXY((Math::randInt()&0xf) - 7, (Math::randInt()&0xf)-7);

        world.addBody(player);
    }*/
    Body player;
    player.bounceFactor.setNumber(0,0);
    player.flyingDrag.setNumber(1,0);
    player.contactDrag.setNumber(0,0);
    player.id = playerId;
    player.spriteX = 104;
    player.spriteY = 0;
    player.spriteW = 8;
    player.spriteH = 8;
    player.bodyHandler = this;
    player.position.setXY(100,100);
    player.sprite = &foreground;
    world.addBody(player);
    world.scene = &scene;
    camera.position = Fixed2D4(100,104);
    renderBuffer.setClearBackground(true,renderBuffer.rgb(80,60,70));
}

void LevelMapScreen::handle(Body *b) {
    if (b->id == playerId) {
        uint8_t tileIndex;
        bool contact = !world.scene->isPixelFree(b->position.x.getIntegerPart()+1, b->position.y.getIntegerPart()+4, tileIndex)
            || !world.scene->isPixelFree(b->position.x.getIntegerPart()-1, b->position.y.getIntegerPart()+4, tileIndex);
        if (Joystick::getJoystick().y < 0) {
            //printf("yo\n");
            if (contact && jumpCounter > 5) {
                jumpCounter = 0;
                b->velocity.y = FixedNumber16<4>(-3,0);
                b->position.y.setFractionPart(0);
            }
        }
        if (contact) {
            b->velocity.x = Joystick::getJoystick().x * 2;
            if (Joystick::getJoystick().x.getFractionPart() != 0)
                b->velocity.y += FixedNumber16<4>(-1,15);
        //    renderBuffer.setClearBackground(true,renderBuffer.rgb(80,60,70));
        } else {
        //    renderBuffer.setClearBackground(true,renderBuffer.rgb(80,60,120));
        }
        camera.position = b->position;
        jumpCounter+=1;
    }
}

void LevelMapScreen::update() {
    static int frame = 0;
    frame += 1;

    /*if (frame % 100 == 0) {
        for (int i=0;i<5;i+=1) {
            Body* player = world.getBody(i);
            player->sprite = TextureData::ztiles_foreground();
            player->spriteW = 8;
            player->spriteH = 8;
            player->spriteX = 104;
            player->spriteY = 0;
            player->position.setXY(110,100);
            player->velocity.randomCircle(FixedNumber16<4>(3,0));
            //player->velocity.setXY((Math::randInt()&7) - 3, (Math::randInt()&7)-3);
        }
    }*/

    Fixed2D4 offset =
        (ScreenButtonState::isButtonOn(SCREENBUTTON_BOTTOMRIGHT) ? Fixed2D4(0,8,0,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_BOTTOMLEFT) ? Fixed2D4(-1,8,0,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_TOPRIGHT) ? Fixed2D4(0,8,-1,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_TOPLEFT) ? Fixed2D4(-1,8,-1,8) : Fixed2D4(0,0));
    //printf("%d %d\n",camera.position.x.getIntegerPart(),camera.position.y.getIntegerPart());
    camera.position += offset + Joystick::getJoystick()*8;
    renderer.update(renderBuffer, scene, camera.position.x.getIntegerPart(), (camera.position.y.getIntegerPart()>>1)+scene.calcHeight()/2,0,1,1);
    renderer.update(renderBuffer, scene, camera.position.x.getIntegerPart(), camera.position.y.getIntegerPart(),1,2,1);
    //world.updateStep(camera);
    Texture<uint16_t> tex = Texture<uint16_t>(ImageAsset::ztiles_foreground);
    //renderBuffer.drawRect(48, 32,32,32)
      //  ->sprite(&texture::beastlands);
    //    ->sprite(&tex,0,0);
        //->filledRect(0);

    Math::Vector2D16 pos(camera.position.x.getIntegerPart(), camera.position.y.getIntegerPart());
    Math::Vector2D16 result = scene.moveOut(pos);
    uint8_t index;
    bool isFree = true; //scene.isPixelFree(pos.x,pos.y,index);
    //tileMap->update(camera);
}
