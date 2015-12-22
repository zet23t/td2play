#include "game.h"
#include "lib_math.h"

void World::init() {
    bodyCount = 0;
}

void World::addBody(Body &body) {
    if (bodyCount >= MAX_BODIES) return;
    body.world = this;
    bodyList[bodyCount++] = body;
}
void World::updateStep(Camera& camera) {
    for (uint8_t i = 0; i < bodyCount;i+=1) {
        bodyList[i].updateStep(camera);
    }
}
bool World::isFree(const Fixed2D4& pos) const {
    if (!scene) return true;
    Math::Vector2D16 p = Math::Vector2D16(pos.x.getIntegerPart(), pos.y.getIntegerPart());
    uint8_t tileType;
    return scene->isPixelFree(p.x,p.y, tileType);
}
Fixed2D4 World::moveOut(const Fixed2D4& pos) const {
    if (!scene) return pos;
    Math::Vector2D16 p = Math::Vector2D16(pos.x.getIntegerPart(), pos.y.getIntegerPart());
    Math::Vector2D16 res = scene->moveOut(p);
    if (p.x == res.x && p.y == res.y) return pos;
    Fixed2D4 correct;
    correct.x.setIntegerPart(res.x);
    correct.y.setIntegerPart(res.y);
    //printf("%s %s\n",correct.x.toString(), correct.y.toString());
    return correct;
}

void Body::updateStep(Camera& camera) {
    int16_t oldPosX = position.x.getIntegerPart();
    int16_t oldPosY = position.y.getIntegerPart();
    velocity = velocity *  FixedNumber16<4>(0,15) + Fixed2D4(0,0,0,4);
    position+= velocity;
    int16_t posX = position.x.getIntegerPart();
    int16_t posY = position.y.getIntegerPart();
    if (posX != oldPosX || posY != oldPosY) {
        Fixed2D4 correct = world->moveOut(position);
        if (position != correct) {
            velocity = -velocity.scale(0,12);
            position.setIntegerPart(correct);
            posX = position.x.getIntegerPart();
            posY = position.y.getIntegerPart();
        }
    }
    int16_t camX = camera.position.x.getIntegerPart();
    int16_t camY = camera.position.y.getIntegerPart();
    renderBuffer.drawRect(posX-camX + 48, posY-camY + 32,1,1)
                                    ->filledRect(renderBuffer.rgb(255,255,30));

}
