#include "screen.h"
#include "physics.h"

class LevelMapScreen : public Screen, BodyHandler {
private:
    TileMap::Scene<uint16_t> scene;
    TileMap::SceneRenderer<uint16_t, 300> renderer;
    World world;
    Camera camera;
    const uint8_t playerId;
    int jumpCounter;
public:
    LevelMapScreen():playerId(1) {

    }
    LevelMapScreen(TileMap::Scene<uint16_t> &scene): scene(scene), world(), camera(),playerId(1), jumpCounter(0) {
    }
    void load(TileMap::Scene<uint16_t>& scene) {
        this->scene = scene;
    }
    void init();
    void update();
    virtual void handle(Body* b);
};
