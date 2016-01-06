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
Body* World::getBody(int i) {
    if (i >= bodyCount) return 0;
    return &bodyList[i];
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
    moveOut(pos,0,0,0,0);
}
Fixed2D4 World::moveOut(const Fixed2D4& pos, uint8_t distleft, uint8_t distright, uint8_t disttop, uint8_t distbottom) const {
    if (!scene) return pos;
    Math::Vector2D16 p = Math::Vector2D16(pos.x.getIntegerPart(), pos.y.getIntegerPart());
    Math::Vector2D16 res = scene->moveOut(p, distleft, distright, disttop, distbottom);
    if (p.x == res.x && p.y == res.y) return pos;
    return Fixed2D4(res.x,res.y);
}

bool Body::checkForCollission(const int8_t relX, const int8_t relY, const int16_t oldFracX, const int16_t oldFracY) {
    Fixed2D4 rel = Fixed2D4(relX,relY);
    Fixed2D4 expected = position + rel;
    Fixed2D4 correct = world->moveOut(position + rel, relX, spriteW - relX, relY, spriteH - relY);
    if (expected != correct) {
        position = correct - rel;
        position.x.setFractionPart(oldFracX);
        position.y.setFractionPart(oldFracY);
        return true;
    }
    return false;
}

void Body::updateStep(Camera& camera) {
    int16_t oldPosX = position.x.getIntegerPart();
    int16_t oldPosY = position.y.getIntegerPart();
    int16_t oldFracX = position.x.getFractionPart();
    int16_t oldFracY = position.y.getFractionPart();
    velocity = velocity *  FixedNumber16<4>(0,15) + Fixed2D4(0,0,0,4);
    position+= velocity;
    int16_t posX = position.x.getIntegerPart();
    int16_t posY = position.y.getIntegerPart();
    if ((posX != oldPosX || posY != oldPosY) && (spriteW > 0 && spriteH > 0)) {
        Fixed2D4 before = position;
        int w = (spriteW - 1) >> 1;
        int h = (spriteH - 1) >> 1;
        bool hit = checkForCollission(0,0,oldFracX, oldFracY)
            || checkForCollission(0,h,oldFracX, oldFracY)
            || checkForCollission(0,-h,oldFracX, oldFracY)
            || checkForCollission(w,0,oldFracX, oldFracY)
            || checkForCollission(-w,0,oldFracX, oldFracY)
            || checkForCollission(w,-h,oldFracX, oldFracY)
            || checkForCollission(-w,-h,oldFracX, oldFracY)
            || checkForCollission(w,h,oldFracX, oldFracY)
            || checkForCollission(-w,h,oldFracX, oldFracY);

        if (hit) {
            posX = position.x.getIntegerPart();
            posY = position.y.getIntegerPart();

            velocity = -velocity.scale(0,12) - (before - position).scale(0,2);
        }

    }
    int16_t camX = camera.position.x.getIntegerPart();
    int16_t camY = camera.position.y.getIntegerPart();
    RenderCommand<uint16_t>* cmd = renderBuffer.drawRect(posX-camX + 48 - (spriteW / 2), posY-camY + 32 - spriteH/ 2,spriteW,spriteH);
    if (sprite) {
        cmd->sprite(sprite, spriteX, spriteY);
    } else {
        cmd->filledRect(renderBuffer.rgb(255,255,30));
    }

}
