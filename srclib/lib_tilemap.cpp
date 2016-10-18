#include "lib_tilemap.h"
#include <stdio.h>

namespace TileMap {
    static bool isRectIntersectingRect(int ax1,int ay1,int ax2,int ay2, int bx1, int by1, int bx2, int by2) {
        return !(ax2 < bx1 || bx2 < ax1 || ay2 < by1 || by2 < ay1);
    }

    template<class TColor>
    bool Scene<TColor>::isRectFree(const int x1, const int y1, const int x2, const int y2) const {
        if (!flagmap.isValid()) return true;
        const uint8_t tileSizeBits = tileset.tileSizeBits;
        const uint8_t tileSize = 1 << tileSizeBits;
        const int16_t tileX1 = x1 >> tileSizeBits;
        const int16_t tileY1 = y1 >> tileSizeBits;
        const int16_t tileX2 = x2 >> tileSizeBits;
        const int16_t tileY2 = y2 >> tileSizeBits;

        const uint8_t width = tilemaps[0].getWidth();
        const uint8_t height = tilemaps[0].getHeight();

        for (int tileX = tileX1;tileX <= tileX2; tileX+=1)
        {
            for (int tileY = tileY1;tileY <= tileY2; tileY+=1)
            {
                if (tileX < 0 || tileY < 0 || tileX >= width || tileY >= height) {
                    continue;
                }

                const uint8_t tileIndex = flagmap.get(tileX,tileY);
                if (tileIndex != 255)
                {
                    int16_t left= tileX << tileSizeBits;
                    int16_t top = tileY << tileSizeBits;
                    switch(tileIndex) {
                    case 0:
                        return false;
                    case 5: // top half blocked
                        //printf("%d,%d : %d,%d  |  %d,%d : %d,%d\n",x1,y1,x2,y2, left,top,left+tileSize,top+tileSize/2);
                        if (isRectIntersectingRect(x1,y1,x2,y2, left,top,left+tileSize,top+tileSize/2)) {
                            return false;
                        }
                        break;
                    case 6: // bottom half blocked
                        if (isRectIntersectingRect(x1,y1,x2,y2, left,top+tileSize/2,left+tileSize,top+tileSize)) {
                            return false;
                        }
                        break;
                    case 7: // left half blocked
                        if (isRectIntersectingRect(x1,y1,x2,y2, left,top,left+tileSize/2,top+tileSize)) {
                            return false;
                        }
                        break;
                    case 8: // right half blocked
                        if (isRectIntersectingRect(x1,y1,x2,y2, left+tileSize/2,top,left+tileSize,top+tileSize)) {
                            return false;
                        }
                        break;
                    default:
                        #ifdef WIN32
                        printf("unhandled flagmap id: %d\n",tileIndex);
                        #endif
                        break;
                    }
                }
            }
        }
        return true;
    }

    template<class TColor>
    int Scene<TColor>::getTileIndex(int x, int y, uint8_t& textureXOut, uint8_t & textureYOut, uint8_t & tilesetIndexOut) const {
        x -= (RenderBufferConst::screenWidth >> 1);
        y -= (RenderBufferConst::screenHeight >> 1);
        const uint8_t tileSizeBits = tileset.tileSizeBits;
        const uint8_t tileSize = 1 << tileSizeBits;
        const uint8_t tileSizeMask = (1 << tileSizeBits) - 1;
        const int16_t tileX = x >> tileSizeBits;
        const int16_t tileY = y >> tileSizeBits;
        textureXOut = x & tileSizeMask;
        textureYOut = y & tileSizeMask;
        for (int i=tilemapCount - 1;i>=0; i-=1) {
            const uint8_t tileIndex = tilemaps[i].get(tileX, tileY);
            if (tileIndex != 255) {
                if (!tileset.tileSets[i].isTransparent(((tileIndex & 0xf) << tileSizeBits) + textureXOut,
                                                       ((tileIndex >> 4) << tileSizeBits) + textureYOut)) {
                    tilesetIndexOut = i;
                    return tileIndex;
                }
            }
        }
        return -1;
    }

    template<class TColor>
    bool Scene<TColor>::findLineIntersection(int x1, int y1, int x2, int y2, int &resultX, int &resultY) const {
        int dx = x2 - x1;
        int dy = y2 - y1;
        const bool followHorizontal = abs(dx) >= abs(dy);

        if ((followHorizontal && x2 < x1) || (!followHorizontal && y2 < y1)) {
            int tmp = x1;
            x1 = x2;
            x2 = tmp;
            tmp = y1;
            y1 = y2;
            y2 = tmp;
            dx = -dx;
            dy = -dy;
        }

        const uint8_t tileSizeBits = tileset.tileSizeBits;
        const uint8_t width = tilemaps[0].getWidth();
        const uint8_t height = tilemaps[0].getHeight();
        const int tileX1 = x1 >> tileSizeBits;
        const int tileX2 = x2 >> tileSizeBits;
        const int tileY1 = y1 >> tileSizeBits;
        const int tileY2 = y2 >> tileSizeBits;

        int x = x1;
        int y = y1;
        int tileX = tileX1;
        int tileY = tileY1;
        do {
            if (tileX >= 0 && tileY >= 0 && tileX < width && tileY < height) {
                const uint8_t tileIndex = flagmap.get(tileX,tileY);
                switch (tileIndex) {
                    case 255: break; // empty
                }
            }
            if (followHorizontal) {

            } else {

            }
        } while (tileX != tileX2 || tileY != tileY2);
        return false;
    }

    template<class TColor>
    bool Scene<TColor>::isPixelFree(const int x, const int y, uint8_t& tileIndexOut) const {
        if (!flagmap.isValid()) return true;
        const uint8_t tileSizeBits = tileset.tileSizeBits;
        const int16_t tileX = x >> tileSizeBits;
        const int16_t tileY = y >> tileSizeBits;
        const uint8_t width = tilemaps[0].getWidth();
        const uint8_t height = tilemaps[0].getHeight();

        if (tileX < 0 || tileY < 0 || tileX >= width || tileY >= height) {
            return true;
        }

        const uint8_t tileIndex = flagmap.get(tileX,tileY);


        /*const uint8_t tileIndex = foreground.get(tileX,tileY);
        if (tileIndex == 255) {
            return true;
        }

        const uint8_t subX = x & ((1<<tileSizeBits)-1);
        const uint8_t subY = y & ((1<<tileSizeBits)-1);
        const bool transparent = tileset.foreground.isTransparent(
                                         ((tileIndex & 0xf) << tileSizeBits) + subX,
                                         ((tileIndex >> 4) << tileSizeBits) + subY);
        if (transparent) return true;
        tileIndexOut = tileIndex;*/
        return tileIndex == 255;
    }

    template<class TColor>
    Math::Vector2D16 Scene<TColor>::moveOut(const Math::Vector2D16& pos, const uint8_t distleft, const uint8_t distright, const uint8_t disttop, const uint8_t distbottom) const {
        uint8_t tileIndex;
        if (isPixelFree(pos.x,pos.y,tileIndex)) return pos;

        uint8_t right = 0, left = 0, up = 0, down = 0;
        uint8_t rightIndex = 0, leftIndex = 0, upIndex = 0, downIndex = 0;
        while (right < 32 && !isPixelFree(pos.x + right, pos.y, rightIndex)) right += 1;
        while (left < 32 && !isPixelFree(pos.x - left, pos.y, leftIndex)) left += 1;
        while (up < 32 && !isPixelFree(pos.x, pos.y - up, upIndex)) up += 1;
        while (down < 32 && !isPixelFree(pos.x, pos.y + down, downIndex)) down += 1;

        if (right - distleft < left - distright && right - distleft < down - disttop && right - distleft < up - distbottom) return pos + Math::Vector2D16(right, 0);
        if (left - distright < down - disttop && left - distright < up - distbottom) return pos + Math::Vector2D16(-left, 0);
        if (up - distbottom <= down - disttop) return pos + Math::Vector2D16(0, -up);
        return pos + Math::Vector2D16(0, down);
    }

    template<class TColor>
    Math::Vector2D16 Scene<TColor>::moveOut(const Math::Vector2D16& pos) const {
        return moveOut(pos,0,0,0,0);
    }
    template class Scene<uint8_t>;
    template class Scene<uint16_t>;
}
