#include "game.h"
#include "tilemap.h"

void LevelMapScreen::update() {
    camera.position += Fixed2D4(0,8,0,8);
    //printf("%s\n",camera.position.x.toString());
    tileMap->update(camera);
}
