#include "game.h"
#include "tilemap.h"
#include "lib_input.h"
#include "lib_tilemap.h"

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
    //tileMap->update(camera);
}
