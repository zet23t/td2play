#ifndef __LIB_TILEMAP_H__
#define __LIB_TILEMAP_H__

#include <inttypes.h>
#include "lib_RenderBuffer.h"
#include "lib_math.h"

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
        ProgmemData():width(0),height(0),widthBits(0),tiles(0) {

        }
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
        inline uint8_t getWidth() const { return width; }
        inline uint8_t getHeight() const { return height; }
        inline uint8_t get(const uint8_t x, const uint8_t y) const {
            return pgm_read_byte_far(&tiles[x + (y << widthBits)]);
        }
        inline uint8_t get(const uint16_t index) const {
            #ifdef WIN32
            if (index < width * height) {
                    assert(index < width * height);
            }
            #endif // WIN32
            return pgm_read_byte_far(&tiles[index]);
        }
        inline bool isValid() const {
            return tiles != 0;
        }
    };

    /**
     * The tileset defines the graphical representation of tile map data.
     */
    template<class TColor>
    class TileSet {
    public:
        Texture<TColor>* tileSets;
        uint8_t tilesetCount;
        uint8_t tileSizeBits;
        TileSet():tileSets(0),tilesetCount(0),tileSizeBits(0) {
        }
        TileSet(Texture<TColor>* tileSets, uint8_t tilesetCount, uint8_t tileSizeBits):
            tileSets(tileSets), tilesetCount(tilesetCount), tileSizeBits(tileSizeBits) {}
    };

    /// All required data for rendering the tilemap
    template<class TColor>
    class Scene {
    public:
        ProgmemData* tilemaps;
        ProgmemData flagmap;
        uint8_t tilemapCount;
        TileSet<TColor> tileset;
        uint8_t *progMemTileTypeFlags;
        Scene() {
        }
        Scene(ProgmemData* tilemaps, uint8_t tilemapCount, TileSet<TColor> tileset, uint8_t* progMemTileTypeFlags):
            tilemaps(tilemaps), flagmap(), tilemapCount(tilemapCount), tileset(tileset), progMemTileTypeFlags(progMemTileTypeFlags) {
            assert(this->tilemaps[0].getWidth() > 0 && tilemaps[0].getHeight() > 0);
            //assert(background.getWidth() == foreground.getWidth() && background.getHeight() == foreground.getHeight());
        }
        Scene& setFlagmap(ProgmemData flagmap) {
            this->flagmap = flagmap;
            return *this;
        }
        uint16_t calcWidth() const {
            return tilemaps[0].getWidth() * (1 << tileset.tileSizeBits);
        }
        uint16_t calcHeight() const {
            return tilemaps[0].getHeight() * (1 << tileset.tileSizeBits);
        }
        bool isPixelFree(const int x, const int y, uint8_t& tileIndex) const;
        bool isRectFree(const int x1, const int y1, const int x2, const int y2) const;
        Math::Vector2D16 moveOut(const Math::Vector2D16& pos) const;
        Math::Vector2D16 moveOut(const Math::Vector2D16& pos, const uint8_t distleft, const uint8_t distright, const uint8_t disttop, const uint8_t distbottom) const;
    };

    template<class TColor, int maxCommands>
    class SceneRenderer {
    private:

    public:
        SceneRenderer()
        {

        }

        void update(RenderBuffer<TColor, maxCommands>& buffer, Scene<TColor>& scene, const int16_t centerX, const int16_t centerY, int startLayer, int layerCount) const;
    };

    template<class TColor, int maxCommands>
    void TileMap::SceneRenderer<TColor, maxCommands>::update(RenderBuffer<TColor, maxCommands>& buffer, Scene<TColor>& scene, const int16_t centerX, const int16_t centerY, int startLayer, int layerCount) const
    {
        const uint8_t tileSizeBits = scene.tileset.tileSizeBits;
        const int16_t topLeftX = (centerX & ~((1<<tileSizeBits)-1)) - (RenderBufferConst::screenWidth >> 1);
        const int16_t topLeftY = (centerY  & ~((1<<tileSizeBits)-1))- (RenderBufferConst::screenHeight >> 1);
        const int16_t minX = topLeftX;
        const int16_t maxX = topLeftX + RenderBufferConst::screenWidth + (1 << tileSizeBits);
        const int16_t minY = topLeftY;
        const int16_t maxY = topLeftY + RenderBufferConst::screenHeight + (1 << tileSizeBits);
        const uint8_t width = scene.tilemaps[0].getWidth();
        const uint8_t height = scene.tilemaps[0].getHeight();

        for (int layerIndex = startLayer; layerIndex < scene.tilemapCount && layerIndex < startLayer + layerCount; layerIndex+=1) {
            for (int16_t y = minY; y < maxY; y += 1 << tileSizeBits) {
                for (int16_t x = minX; x < maxX; x += 1 << tileSizeBits) {
                    if (y < 0 || x < 0 || x >= width << tileSizeBits || y >= height << tileSizeBits) continue;
                    const uint8_t tileX = x >> tileSizeBits;
                    const uint8_t tileY = y >> tileSizeBits;
                    const uint16_t index = tileX + tileY * width;
                    const int8_t rectX = x + (RenderBufferConst::screenWidth>>1) - centerX;
                    const int8_t rectY = y + (RenderBufferConst::screenHeight>>1) - centerY;
                    const uint8_t tileIndex = scene.tilemaps[layerIndex].get(index);
                    if (tileIndex != 0xff) {
                        buffer.drawRect(rectX, rectY,8,8)
                                          ->sprite(&scene.tileset.tileSets[layerIndex],
                                                   (tileIndex & 0xf) << tileSizeBits,
                                                   (tileIndex >> 4) << tileSizeBits);
                    }
                }
            }
        }
    }

}
#endif // __LIB_TILEMAP_H__
