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
            assert(width>0 && height>0);
            widthBits = 0xff;
            for (uint8_t i=1, bit = 0;i<=width;i*=2, bit+=1) {
                if (i == width) {
                    widthBits = bit;
                    break;
                }
            }
            assert(widthBits != 0xff);
        }
        inline uint8_t getWidth() const { assert(width > 0); return width; }
        inline uint8_t getHeight() const { return height; }
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
            assert(background.getWidth() > 0 && background.getHeight() > 0);
            assert(background.getWidth() == foreground.getWidth() && background.getHeight() == foreground.getHeight());
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

    template<class TColor, int maxCommands>
    void TileMap::SceneBgFgRenderer<TColor, maxCommands>::update(RenderBuffer<TColor, maxCommands>& buffer, SceneBgFg<TColor>& scene, const int16_t centerX, const int16_t centerY) const
    {
        const uint8_t tileSizeBits = scene.tileset.tileSizeBits;
        const int16_t topLeftX = (centerX & ~((1<<tileSizeBits)-1)) - (RenderBufferConst::screenWidth >> 1);
        const int16_t topLeftY = (centerY  & ~((1<<tileSizeBits)-1))- (RenderBufferConst::screenHeight >> 1);
        const int16_t minX = topLeftX;
        const int16_t maxX = topLeftX + RenderBufferConst::screenWidth + (1 << tileSizeBits);
        const int16_t minY = topLeftY;
        const int16_t maxY = topLeftY + RenderBufferConst::screenHeight + (1 << tileSizeBits);
        const uint8_t width = scene.background.getWidth();
        const uint8_t height = scene.background.getHeight();
printf("%d %d\n%d %d\n %d %d\n",minX,maxX,minY,maxY, width,height);

        for (int16_t y = minY; y < maxY; y += 1 << tileSizeBits) {
            for (int16_t x = minX; x < maxX; x += 1 << tileSizeBits) {
                if (y < 0 || x < 0 || x >= width << tileSizeBits || y >= height << tileSizeBits) continue;
                const uint8_t tileX = x >> tileSizeBits;
                const uint8_t tileY = y >> tileSizeBits;
                const uint16_t index = tileX + tileY * width;
                const int8_t rectX = x + (RenderBufferConst::screenWidth>>1) - centerX;
                const int8_t rectY = y + (RenderBufferConst::screenHeight>>1) - centerY;
                const uint8_t tileIndexBg = scene.background.get(index);
                const uint8_t tileIndexFg = scene.foreground.get(index);
                if (tileIndexBg != 0xff)
                    buffer.drawRect(rectX, rectY,8,8)
                                      ->sprite(&scene.tileset.background,
                                               (tileIndexBg & 0xf) << tileSizeBits,
                                               (tileIndexBg >> 4) << tileSizeBits);
                if (tileIndexFg != 0xff)
                    buffer.drawRect(rectX, rectY,8,8)
                                      ->sprite(&scene.tileset.foreground,
                                               (tileIndexFg & 0xf) << tileSizeBits,
                                               (tileIndexFg >> 4) << tileSizeBits);
                                      //->filledRect(renderBuffer.rgb(255&(x+64),255&(y+64),0));
            }
        }
    }

}
#endif // __LIB_TILEMAP_H__
