#include "lib_tilemap.h"

namespace TileMap {

    template<class TColor>
    Math::Vector2D16 SceneBgFg<TColor>::moveOut(const Math::Vector2D16& pos) {
        return pos;
    }
    template class SceneBgFg<uint8_t>;
    template class SceneBgFg<uint16_t>;
}
