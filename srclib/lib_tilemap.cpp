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
        tileIndexOut = tileIndex;
        return false;
    }

    template<class TColor>
    Math::Vector2D16 SceneBgFg<TColor>::moveOut(const Math::Vector2D16& pos) const {
        uint8_t tileIndex;
        if (isPixelFree(pos.x,pos.y,tileIndex)) return pos;

        uint8_t right = 0, left = 0, up = 0, down = 0;
        uint8_t rightIndex = 0, leftIndex = 0, upIndex = 0, downIndex = 0;
        while (right < 32 && !isPixelFree(pos.x + right, pos.y, rightIndex)) right += 1;
        while (left < 32 && !isPixelFree(pos.x - left, pos.y, leftIndex)) left += 1;
        while (up < 32 && !isPixelFree(pos.x, pos.y - up, upIndex)) up += 1;
        while (down < 32 && !isPixelFree(pos.x, pos.y + down, downIndex)) down += 1;

        if (right < left && right < down && right < up) return pos + Math::Vector2D16(right, 0);
        if (left < down && left < up) return pos + Math::Vector2D16(-left, 0);
        if (up < down) return pos + Math::Vector2D16(0, -up);
        return pos + Math::Vector2D16(0, down);
    }
    template class SceneBgFg<uint8_t>;
    template class SceneBgFg<uint16_t>;
}
