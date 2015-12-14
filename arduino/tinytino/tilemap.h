#ifndef __TILEMAP_H__
#define __TILEMAP_H__

class TileMap {
private:
    uint8_t width;
    uint8_t height;
    uint8_t tileSizeBits;
    uint16_t *tiles;
    int16_t offsetX, offsetY;
    uint8_t paralax;
public:
    TileMap(uint8_t width, uint8_t height, uint8_t tileSizeBits, int16_t offsetX, int16_t offsetY, uint8_t paralax, uint16_t *tiles)
        : width(width), height(height), tileSizeBits(tileSizeBits),
        offsetX(offsetX), offsetY(offsetY), paralax(paralax), tiles(tiles)
    {

    }
    void update(const Camera &camera) {
        const int16_t centerX = camera.position.x.getIntegerPart();
        const int16_t centerY = camera.position.y.getIntegerPart();
        int16_t topLeftX = (centerX & ~((1<<tileSizeBits)-1)) - (RenderBufferConst::screenWidth >> 1);
        int16_t topLeftY = (centerY  & ~((1<<tileSizeBits)-1))- (RenderBufferConst::screenHeight >> 1);
        const int16_t minX = topLeftX;
        const int16_t maxX = topLeftX + RenderBufferConst::screenWidth + (1 << tileSizeBits);
        const int16_t minY = topLeftY;
        const int16_t maxY = topLeftY + RenderBufferConst::screenHeight + (1 << tileSizeBits);

        for (int16_t y = minY; y < maxY; y += 1 << tileSizeBits) {
            for (int16_t x = minX; x < maxX; x += 1 << tileSizeBits) {
                if (y < 0 || x < 0 || x >= width << tileSizeBits || y >= height << tileSizeBits) continue;
                const uint8_t tileX = x >> tileSizeBits;
                const uint8_t tileY = y >> tileSizeBits;
                const uint16_t tileIndex = tiles[tileX + tileY * width];

                renderBuffer.drawRect(x + (RenderBufferConst::screenWidth>>1) - centerX,
                                      y + (RenderBufferConst::screenHeight>>1) - centerY,8,8)
                                      ->sprite(&texture::beastlands,
                                               (tileIndex & 0xf) << tileSizeBits,
                                               (tileIndex >> 4) << tileSizeBits);
                                      //->filledRect(renderBuffer.rgb(255&(x+64),255&(y+64),0));
            }
        }

    }
};


extern uint16_t data_level[];
TileMap map_0(128,32,3,0,0,0,data_level);

#endif // __TILEMAP_H__
