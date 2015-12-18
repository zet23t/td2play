#include "lib_tilemap.h"

template<class TColor, int maxCommands>
void TileDataMap<TColor, maxCommands>::update(RenderBuffer<TColor, maxCommands>& buffer, SceneBgFg<TColor>& scene, const int16_t centerX, const int16_t centerY) const
{
    const uint8_t tileSizeBits = tileset.tileSizeBits;
    const int16_t topLeftX = (centerX & ~((1<<tileSizeBits)-1)) - (RenderBufferConst::screenWidth >> 1);
    const int16_t topLeftY = (centerY  & ~((1<<tileSizeBits)-1))- (RenderBufferConst::screenHeight >> 1);
    const int16_t minX = topLeftX;
    const int16_t maxX = topLeftX + RenderBufferConst::screenWidth + (1 << tileSizeBits);
    const int16_t minY = topLeftY;
    const int16_t maxY = topLeftY + RenderBufferConst::screenHeight + (1 << tileSizeBits);

    for (int16_t y = minY; y < maxY; y += 1 << tileSizeBits) {
        for (int16_t x = minX; x < maxX; x += 1 << tileSizeBits) {
            if (y < 0 || x < 0 || x >= width << tileSizeBits || y >= height << tileSizeBits) continue;
            const uint8_t tileX = x >> tileSizeBits;
            const uint8_t tileY = y >> tileSizeBits;
            const uint16_t index = tileX + tileY * width;
            const int8_t rectX = x + (RenderBufferConst::screenWidth>>1) - centerX;
            const int8_t rectY = y + (RenderBufferConst::screenHeight>>1) - centerY;
            const uint8_t tileIndexBg = backgroundTileMap[index];
            const uint8_t tileIndexFg = backgroundTileMap[index];

            if (tileIndexBg != 0xff)
                buffer.drawRect(rectX, rectY,8,8)
                                  ->sprite(backgroundTexture,
                                           (tileIndexBg & 0xf) << tileSizeBits,
                                           (tileIndexBg >> 4) << tileSizeBits);
            if (tileIndexFg != 0xff)
                buffer.drawRect(rectX, rectY,8,8)
                                  ->sprite(foregroundTexture,
                                           (tileIndexFg & 0xf) << tileSizeBits,
                                           (tileIndexFg >> 4) << tileSizeBits);
                                  //->filledRect(renderBuffer.rgb(255&(x+64),255&(y+64),0));
        }
    }
}
