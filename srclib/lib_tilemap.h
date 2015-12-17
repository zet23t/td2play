#ifndef __LIB_TILEMAP_H__
#define __LIB_TILEMAP_H__

#include <inttypes.h>
#include "lib_RenderBuffer.h"

template<class TColor, int maxCommands>
class TileDataMap {
private:
    // Not sure if packing back- and foreground into same class is wise
    // could also create tilemap class, but sharing calculations would be
    // difficult
    Texture<TColor> backgroundTexture;
    Texture<TColor> foregroundTexture;
    uint16_t width, height;
    const char *backgroundTileMap;
    const char *foregroundTileMap;
public:
    TileDataMap(Texture<TColor> backgroundTexture,
                Texture<TColor> foregroundTexture,
                uint16_t width, uint16_t height,
                const char *backgroundTileMap,
                const char *foregroundTileMap):
                    backgroundTexture(backgroundTexture),
                    foregroundTexture(foregroundTexture),
                    width(width), height(height),
                    backgroundTileMap(backgroundTileMap),
                    foregroundTileMap(foregroundTileMap)
    {

    }

    void update(RenderBuffer<TColor, maxCommands>& buffer, const uint16_t cameraX, const uint16_t cameraY) const;
};

#endif // __LIB_TILEMAP_H__
