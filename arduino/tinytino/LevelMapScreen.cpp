#include "game.h"
#include "tilemap.h"
#include "lib_input.h"
#include "lib_tilemap.h"
#include "lib_math.h"

void LevelMapScreen::init() {
    camera.position = Fixed2D4(100,104);
    renderBuffer.setClearBackground(true,renderBuffer.rgb(80,60,70));
}

void LevelMapScreen::update() {
    Fixed2D4 offset =
        (ScreenButtonState::isButtonOn(SCREENBUTTON_BOTTOMRIGHT) ? Fixed2D4(0,8,0,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_BOTTOMLEFT) ? Fixed2D4(-1,8,0,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_TOPRIGHT) ? Fixed2D4(0,8,-1,8) : Fixed2D4(0,0)) +
        (ScreenButtonState::isButtonOn(SCREENBUTTON_TOPLEFT) ? Fixed2D4(-1,8,-1,8) : Fixed2D4(0,0));

    camera.position += offset + Joystick::getJoystick()*8;
    renderer.update(renderBuffer, scene, camera.position.x.getIntegerPart(), camera.position.y.getIntegerPart());

    Math::Vector2D16 pos(camera.position.x.getIntegerPart(), camera.position.y.getIntegerPart());
    //Math::Vector2D16 result = scene.moveOut(pos);
    uint8_t index;
    bool isFree = scene.isPixelFree(pos.x,pos.y,index);
    renderBuffer.drawRect(pos.x - pos.x + 48, pos.y - pos.y + 32,1,1)
                                    ->filledRect(renderBuffer.rgb(isFree ? 255 : 0,90,30));
    //tileMap->update(camera);
}
