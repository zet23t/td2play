#include "lib_tilemap.h"
#include <stdio.h>

namespace TileMap {
    template<class TColor>
    bool SceneBgFg<TColor>::isPixelFree(const int x, const int y, uint8_t& tileIndexOut) const {
        const uint8_t tileSizeBits = tileset.tileSizeBits;
        const int16_t tileX = x >> tileSizeBits;
        const int16_t tileY = y >> tileSizeBits;
        const uint8_t width = background.getWidth();
        const uint8_t height = background.getHeight();

        if (tileX < 0 || tileY < 0 || tileX >= width || tileY >= height) {
            return true;
        }

        const uint8_t tileIndex = foreground.get(tileX,tileY);
        if (tileIndex == 255) {
            return true;
        }

        const uint8_t subX = x & ((1<<tileSizeBits)-1);
        const uint8_t subY = y & ((1<<tileSizeBits)-1);
        const bool transparent = tileset.foreground.isTransparent(
                                         ((tileIndex & 0xf) << tileSizeBits) + subX,
                                         ((tileIndex >> 4) << tileSizeBits) + subY);
        if (transparent) return true;
        return false;
    }

    template<class TColor>
    Math::Vector2D16 SceneBgFg<TColor>::moveOut(const Math::Vector2D16& pos) const {
        uint8_t tileIndex;
        if (isPixelFree(pos.x,pos.y,tileIndex)) return pos;

        // there's a hit

        return pos + Math::Vector2D16(2,2);
    }
    template class SceneBgFg<uint8_t>;
    template class SceneBgFg<uint16_t>;
}
