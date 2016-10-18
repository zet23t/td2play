#ifndef __LIB_TILEMAP_H__
#define __LIB_TILEMAP_H__

#include <inttypes.h>
#include "lib_geom.h"
#include "lib_RenderBuffer.h"
#include "lib_math.h"

#ifndef WIN32
#include <avr/pgmspace.h>
#else
#define pgm_read_byte_far(x) *(x)
#endif // WIN32

namespace TileMap {
    template<class TColor> class Scene;

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

    const uint8_t INFO = 1;
    const uint8_t ZONE_SPAWN = 2;
    const uint8_t TRANSITION = 3;
    const uint8_t NPC_SPAWN = 4;
    const uint8_t NPC_WAYPOINT = 5;
    const uint8_t NPC_DESPAWN = 6;
    const uint8_t CUSTOM = 7;
    const uint8_t ZONE_ID = 8;

    struct RectObject {
    public:
        const int16_t x1, y1, x2, y2;
        union {
            const char *name;
            Scene<uint16_t> (*sceneGetter)(void);
            uint32_t id;
        };
        union {
            struct {
                uint8_t paramValueA;
                uint8_t paramValueB;
            };
            struct {
                uint8_t npcSpawnCount;
                uint8_t npcSpawnType;
            };
            struct {
                uint8_t customId;
            };
        };
        uint8_t type;
        bool isRectIntersecting(int16_t rx1, int16_t ry1, int16_t rx2, int16_t ry2) const {
            return !(x1 >= rx2 || rx1 >= x2 || y1 >= ry2 || ry1 >= y2);
        }
    };

    class ObjectGroup {
    private:
        const RectObject* rectObjectList;
        const uint8_t objectListLength;
    public:
        ObjectGroup(const RectObject* rectObjectList, const uint8_t objectListLength)
            :rectObjectList(rectObjectList), objectListLength(objectListLength) {
        };
        const RectObject* first() const {
            return rectObjectList;
        }
        bool next(const RectObject*& element) const {
            if (!element) element = first();
            else element += 1;
            return (int)(element - rectObjectList) < objectListLength;
        }
        bool findRectIntersection(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const RectObject*& hit, uint8_t& offset) const {
            for (;offset < objectListLength; offset +=1) {
                if (rectObjectList[offset].isRectIntersecting(x1,y1,x2,y2)) {
                    hit = &rectObjectList[offset];
                    offset += 1;
                    return true;
                }
            }
            return false;
        };
        bool findRectIntersections(Geom::Rect<int16_t> &a, Geom::Rect<int16_t> &b, const RectObject*& hit, uint8_t& offset, bool& hitA, bool& hitB) const {
            for (;offset < objectListLength; offset +=1) {
                hitA = rectObjectList[offset].isRectIntersecting(a.x1,a.y1,a.x2,a.y2);
                hitB = rectObjectList[offset].isRectIntersecting(b.x1,b.y1,b.x2,b.y2);
                if (hitA || hitB) {
                    hit = &rectObjectList[offset];
                    offset += 1;
                    return true;
                }
            }
            return false;
        };
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
        const ObjectGroup *objectGroup;
        uint16_t nameId;
        Scene() {
        }
        Scene(ProgmemData* tilemaps, uint8_t tilemapCount, TileSet<TColor> tileset, uint8_t* progMemTileTypeFlags):
            tilemaps(tilemaps), flagmap(), tilemapCount(tilemapCount), tileset(tileset), objectGroup(0),nameId(0) {
            assert(this->tilemaps[0].getWidth() > 0 && tilemaps[0].getHeight() > 0);
            //assert(background.getWidth() == foreground.getWidth() && background.getHeight() == foreground.getHeight());
        }
        Scene& setName(const uint16_t nameId) {
            this->nameId = nameId;
            return *this;
        }
        Scene& setObjectGroup(const ObjectGroup *g) {
            objectGroup = g;
            return *this;
        }
        Scene& setFlagmap(ProgmemData flagmap) {
            this->flagmap = flagmap;
            return *this;
        }
        bool findRectIntersection(int16_t x1, int16_t y1, int16_t x2, int16_t y2, const RectObject*& hit, uint8_t& offset) const {
            if (objectGroup) return objectGroup->findRectIntersection(x1,y1,x2,y2,hit,offset);
            return false;
        }
        bool findRectIntersections(Geom::Rect<int16_t>& a, Geom::Rect<int16_t>& b, const RectObject*& hit, uint8_t& offset, bool& hitA, bool& hitB) const {
            if (objectGroup) return objectGroup->findRectIntersections(a,b,hit,offset,hitA,hitB);
            return false;
        }
        bool nextRect(const RectObject*& element) {
            if (objectGroup) return objectGroup->next(element);
            return 0;
        }
        uint16_t calcWidth() const {
            return tilemaps[0].getWidth() * (1 << tileset.tileSizeBits);
        }
        uint16_t calcHeight() const {
            return tilemaps[0].getHeight() * (1 << tileset.tileSizeBits);
        }
        bool findLineIntersection(int x1, int y1, int x2, int y2, int &resultX, int &resultY) const;
        bool isPixelFree(const int x, const int y, uint8_t& tileIndex) const;
        bool isRectFree(const int x1, const int y1, const int x2, const int y2) const;
        int getTileIndex(const int x, const int y, uint8_t& textureXOut, uint8_t & textureYOut, uint8_t & tilesetIndexOut) const;
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

        void update(RenderBuffer<TColor, maxCommands>& buffer, Scene<TColor>& scene, const int16_t centerX, const int16_t centerY, int startLayer, int layerCount, uint8_t depth) const;
    };

    template<class TColor, int maxCommands>
    void TileMap::SceneRenderer<TColor, maxCommands>::update(RenderBuffer<TColor, maxCommands>& buffer, Scene<TColor>& scene, int16_t centerX, int16_t centerY, int startLayer, int layerCount, uint8_t depth) const
    {
        const uint8_t tileSizeBits = scene.tileset.tileSizeBits;
        const uint8_t tileSize = 1 << tileSizeBits;
        const int16_t offsetX = buffer.getOffsetX();
        const int16_t offsetY = buffer.getOffsetY();
        // this offsetting is damn stupid, I shouldn't have one it this way
        const int16_t topLeftX = ((centerX + offsetX) & ~((1<<tileSizeBits)-1)) - (RenderBufferConst::screenWidth >> 1);
        const int16_t topLeftY = ((centerY + offsetY) & ~((1<<tileSizeBits)-1)) - (RenderBufferConst::screenHeight >> 1);
        const int16_t minX = topLeftX;
        const int16_t maxX = topLeftX + RenderBufferConst::screenWidth + (1 << tileSizeBits);
        const int16_t minY = topLeftY;
        const int16_t maxY = topLeftY + RenderBufferConst::screenHeight + (1 << tileSizeBits);
        const uint16_t width = scene.tilemaps[0].getWidth();
        const uint16_t height = scene.tilemaps[0].getHeight();
        for (int layerIndex = startLayer; layerIndex < scene.tilemapCount && layerIndex < startLayer + layerCount; layerIndex+=1) {
            for (int16_t y = minY; y < maxY; y += 1 << tileSizeBits) {
                for (int16_t x = minX; x < maxX; x += 1 << tileSizeBits) {
                    if (y < 0 || x < 0 || x >= width << tileSizeBits || y >= height << tileSizeBits) continue;
                    const uint8_t tileX = x >> tileSizeBits;
                    const uint8_t tileY = y >> tileSizeBits;
                    const int16_t rectX = x + (RenderBufferConst::screenWidth>>1) - centerX;
                    const int16_t rectY = y + (RenderBufferConst::screenHeight>>1) - centerY;
                    const uint8_t tileIndex = scene.tilemaps[layerIndex].get(tileX,tileY);
                    if (tileIndex != 0xff) {
                        buffer.drawRect(rectX, rectY,tileSize,tileSize)
                                          ->sprite(&scene.tileset.tileSets[layerIndex],
                                                   (tileIndex & 0xf) << tileSizeBits,
                                                   (tileIndex >> 4) << tileSizeBits)->setDepth(depth);
                    }
                }
            }
        }
    }

}
#endif // __LIB_TILEMAP_H__
