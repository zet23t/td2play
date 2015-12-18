#ifndef __LIB_TILEMAP_H__
#define __LIB_TILEMAP_H__

#include <inttypes.h>
#include "lib_RenderBuffer.h"

#ifndef WIN32
#include <avr/pgmspace.h>
#else
#define pgm_read_byte_far(x) *(x)
#endif // WIN32

namespace TileMap {

    /**
     * A tile is addressed by an 8 bit value where 255 means "nothing" and
     * every other value is to be mapped on a graphical tile.
     * This class makes tile data available that's stored in program memory.
     * Width and height must be power of two.
     */
    class ProgmemData {
    private:
        uint8_t width, height;
        uint8_t widthBits;
        const uint8_t *tiles;
    public:
        ProgmemData(uint8_t width, uint8_t height, const uint8_t *tiles):
            width(width),height(height),tiles(tiles)
        {
            widthBits = 0xff;
            for (uint8_t i=1, bit = 0;i<=width;i*=2, bit+=1) {
                if (i == width) {
                    widthBits = bit;
                    break;
                }
            }
            assert(widthBits != 0xff);
        }
        inline uint8_t get(const uint8_t x, const uint8_t y) const {
            return pgm_read_byte_far(&tiles[x + (y << widthBits)]);
        }
        inline uint8_t get(const uint16_t index) const {
            #ifdef WIN32
            assert(index < width * height);
            #endif // WIN32
            return pgm_read_byte_far(&tiles[index]);
        }
    };

    /**
     * The tileset defines the graphical representation of tile map data.
     */
    template<class TColor>
    class TileSetBgFg {
    public:
        Texture<TColor> background;
        Texture<TColor> foreground;
        uint8_t tileSizeBits;
        TileSetBgFg(Texture<TColor> background, Texture<TColor> foreground, uint8_t tileSizeBits):
            background(background), foreground(foreground), tileSizeBits(tileSizeBits) {}
    };

    /// All required data for rendering the tilemap
    template<class TColor>
    class SceneBgFg {
    public:
        ProgmemData background;
        ProgmemData foreground;
        TileSetBgFg<TColor> tileset;
        SceneBgFg(ProgmemData background, ProgmemData foreground, TileSetBgFg<TColor> tileset):
            background(background), foreground(foreground), tileset(tileset) {
            assert(background.width == foreground.width && background.height == foreground.height);
        }
    };



    template<class TColor, int maxCommands>
    class SceneBgFgRenderer {
    public:
        SceneBgFgRenderer()
        {

        }

        void update(RenderBuffer<TColor, maxCommands>& buffer, SceneBgFg<TColor>& scene, const int16_t centerX, const int16_t centerY) const;
    };


}
#endif // __LIB_TILEMAP_H__
