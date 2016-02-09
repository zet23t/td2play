#ifndef __SPRITE_FONT_H__
#define __SPRITE_FONT_H__

#include "lib_RenderBuffer.h"

struct SpriteGlyph {
    char letter;
    uint8_t u,v,w,h,spacing,offsetX, offsetY;
};
template<class TColor, int cmdCount>
class SpriteFont {
private:
    const SpriteGlyph *glyphs;
    int glyphCount;
    Texture<TColor> texture;
    int lineHeight;

public:
    SpriteFont(){}
    SpriteFont(const SpriteGlyph *glyphs, int glyphCount, Texture<TColor> texture): glyphs(glyphs), glyphCount(glyphCount), texture(texture){

    }
    void drawText(RenderBuffer<TColor,cmdCount> &buffer, const char* text, int x, int y, int width, int hAlign) {
        int len = strlen(text);
        const SpriteGlyph *glyphList[len];

        int n = 0;
        int height = 0;
        int lineCount = 0;

        while (char c = *(text++)) {
            for (int i=0;i<glyphCount;i+=1) {
                if (c == '\n') {
                    lineCount+=1;
                    glyphList[n++] = 0;
                    continue;
                }
                if (glyphs[i].letter == c) {
                    glyphList[n++] = &glyphs[i];
                }
            }
        }
        int cursorX = x;
        int cursorY = y;
        for (int i=0;i<n;i+=1) {
            const SpriteGlyph *g = glyphList[i];
            if (g == 0) {
                cursorX = 0;
                cursorY += lineHeight;
                continue;
            }
            buffer.drawRect(cursorX, cursorY,g->w,g->h)
                              ->sprite(&texture,g->u,g->v);
            cursorX += g->spacing;
        }
    }
};


#endif // __SPRITE_FONT_H__
