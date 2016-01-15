#include "screen.h"
#include "physics.h"

class LevelMapScreen : public Screen, BodyHandler {
private:
    TileMap::SceneBgFg<uint16_t> scene;
    TileMap::SceneBgFgRenderer<uint16_t, 200> renderer;
    World world;
    Camera camera;
    const uint8_t playerId = 1;
    int jumpCounter;
public:
    LevelMapScreen() {

    }
    LevelMapScreen(TileMap::SceneBgFg<uint16_t> &scene): scene(scene), world(), camera(), jumpCounter(0) {
    }
    void load(TileMap::SceneBgFg<uint16_t>& scene) {
        this->scene = scene;
    }
    void init();
    void update();
    virtual void handle(Body* b);
};
