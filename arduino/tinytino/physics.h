#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <inttypes.h>

#include "lib_FixedMath.h"
#include "lib_RenderBuffer.h"
#include "lib_tilemap.h"

#include "game_types.h"


#define MAX_BODIES 64


class World;
class Camera;
class BodyHandler;

class Body {
private:
    bool checkForCollission(const int8_t relX, const int8_t relY, const int16_t oldFracX, const int16_t oldFracY);
public:
    BodyHandler* bodyHandler;
    World *world;
    FixedNumber16<4> flyingDrag;
    FixedNumber16<4> contactDrag;
    FixedNumber16<4> bounceFactor;
    Fixed2D4 position;
    Fixed2D4 velocity;
    const Texture<uint16_t>* sprite;
    uint8_t spriteX;
    uint8_t spriteY;
    uint8_t spriteW;
    uint8_t spriteH;
    uint8_t id;
    Body():bodyHandler(0), world(0),flyingDrag(0,15),contactDrag(0,5),bounceFactor(0,8),position(0,0), velocity(0,0),sprite(0),spriteX(0),spriteY(0),spriteW(1),spriteH(1), id(0) {}
    void updateStep(Camera &camera);
};

class BodyHandler {
public:
    virtual void handle(Body *b) {};
};


class Camera : public Body {
public:
};

class World {
public:
    TileMap::Scene<uint16_t> *scene;
    Body bodyList[MAX_BODIES];
    uint8_t bodyCount;

    World():scene(0),bodyCount(0){}
    void init();
    void addBody(Body &body);
    Body* getBody(int i);
    Body* getBodyById(int i);
    void updateStep(Camera &camera);
    bool isFree(const Fixed2D4& pos) const;
    Fixed2D4 moveOut(const Fixed2D4& pos) const;
    Fixed2D4 moveOut(const Fixed2D4& pos, uint8_t distleft, uint8_t distright, uint8_t disttop, uint8_t distbottom) const;
};






#endif // __PHYSICS_H__
