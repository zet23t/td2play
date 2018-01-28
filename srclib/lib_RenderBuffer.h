#ifndef __RENDERBUFFER_H__
#define __RENDERBUFFER_H__

#include <inttypes.h>
#include <TinyScreen.h>
#include <assert.h>
#include <stdio.h>
#include "lib_geom.h"
#include "lib_font_virtualdj.h"
#include "lib_StringBuffer.h"
#include "lib_image.h"
#include "lib_spritefont_structs.h"

namespace RenderBufferConst {
    const uint8_t screenWidth = 96;
    const uint8_t screenHeight = 64;
}

namespace RenderCommandType {
    const uint8_t solid = 1;
    const uint8_t text = 2;
    const uint8_t textured = 4;
}

namespace RenderCommandBlendMode {
    const uint8_t opaque = 0;
    const uint8_t bitwiseOr = 1;
    const uint8_t bitwiseAnd = 2;
    const uint8_t average = 3;
    const uint8_t subtract = 4;
    const uint8_t add = 5;
}

namespace TextureType {
    const uint8_t rgb565sram = 1;
    const uint8_t rgb565progmem = 2;
    const uint8_t rgb233sram = 3;
    const uint8_t rgb233progmem = 4;
}
// bits rrrrrggg gggbbbbb expected
//      gggrrrrr bbbbbGGG as is (G is high bit)
// Creates a RGB-565 encoded two byte sequence. The encoding is more difficult than I thought it would be due
// to endian encoding. I don't know how to write this in a better way.
#define RGB565(r,g,b) (uint16_t)(((unsigned char)(r) >> 3) << 8 | ((unsigned char)(g) >> 5 | ((unsigned char)(g) >> 3 & 3) << 13) | ((unsigned char)(b) & ~15))
#define RGB565RAW(r,g,b) (uint16_t)(((unsigned char)(r)) << 8 | ((unsigned char)(g) >> 3 | ((unsigned char)(g) & 7) << 13) | ((unsigned char)(b) << 3))
#define RGB565_TO_RED(col) ((((col) >> 8) & 31))
#define RGB565_TO_GREEN(col) (((col)>>13&7)|((col)&7)<<3)
#define RGB565_TO_BLUE(col) (((((col) >> 3 & 31))))
#define RGB565_MASK_R 0x1f00
#define RGB565_MASK_G 0xe007
#define RGB565_MASK_B 0xf8

//(((((col) & 7 << 5)) | ((((col) >> 13 << 2)) | ((col)&7>>1))))
// The following macro is the version I used first but it produced wrong coloring
//#define RGB565(r,g,b) (uint16_t)(((r) << 8 & 0xf800) | ((g) << 5 & 0x7e0) | ((b) >> 3))

#define RGB233(r,g,b) (((r) >> 6) | ((g) >> 3 & 034) | ((b) & 0340))

#ifdef WIN32
#include <memory.h>
#define pgm_read_byte *
#define pgm_read_word *

#endif

#ifndef PROGMEM
#define PROGMEM
#endif

template<class TColor>
class Texture {
private:
    union {
        const uint8_t *data;
        const uint16_t *rgb565;
        const uint8_t *rgb233;
    };
    void fillLineRgb565 (bool sram, TColor *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v, bool mirrorh, bool mirrorv, uint8_t width, uint8_t blendMode,uint8_t *depthBuffer, uint8_t depth) const;
    void fillLineRgb233sram (TColor *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v,bool mirrorh, bool mirrorv, uint8_t width, uint8_t blendMode,uint8_t *depthBuffer, uint8_t depth) const;
    void fillLineRgb233progmem (TColor *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v,bool mirrorh, bool mirrorv, uint8_t width, uint8_t blendMode,uint8_t *depthBuffer, uint8_t depth) const;
    void init(const uint8_t *data, uint8_t type, uint16_t width, uint16_t height, uint16_t transparentColorMask);
public:
    uint8_t type;
    uint16_t width;
    uint16_t height;
    uint16_t widthMod;
    uint16_t heightMod;
    uint16_t transparentColorMask;
    uint8_t widthBits;
    Texture():data(0),type(0),width(0),height(0),widthMod(0),heightMod(0),transparentColorMask(0),widthBits(0){
    }
    Texture(int):data(0),type(0),width(0),height(0),widthMod(0),heightMod(0),transparentColorMask(0),widthBits(0){
    }
    Texture (const uint8_t *data, uint8_t type, uint16_t width, uint16_t height, uint16_t transparentColorMask);
    Texture (const ImageData& data);
    Texture (const ImageData* data);
    uint8_t getType() const { return type; }
    void fillLine(TColor *lineBuffer, uint8_t lineX, uint8_t u, uint8_t v, bool mirrorh, bool mirrorv, uint8_t width, uint8_t blendMode,uint8_t *depthBuffer, uint8_t depth) const;
    bool isTransparent(uint16_t x, uint16_t y) const;
    TColor getColor(uint16_t x, uint16_t y) const;
};

namespace RenderCommandFlag {
    extern const uint8_t MIRROR_HORIZONTAL;
    extern const uint8_t MIRROR_VERTICAL;
}
namespace RenderCommandData {
    template<class TColor>
    struct Rect {
        union {
            /**
             * RGB-565 / RGB-233 color for the command
             */
            TColor color;
            const Texture<TColor> *texture;
        };
        uint8_t x1, x2;
        uint8_t u, v;
        uint8_t blendMode;
        uint8_t flags;
    };
    template<class TColor>
    struct Text {
        /**
         * RGB-565 / RGB-233 color for the command
         */
        TColor color;
        int16_t x1;
        const FONT_INFO *font;
        const char *text;
    };
}

/**
 * A render command is an instruction to draw something.
 * A command can encode various types of drawing instructions like
 * drawing a filled rect or a text.
 */
template <class TColor>
class RenderCommand
{
public:
    /**
     * The type of the render command
     */
    uint8_t type, depth;
    int8_t y1, y2;
    union
    {
        RenderCommandData::Rect<TColor> rect;
        RenderCommandData::Text<TColor> text;
    };

private:
    /**
     * Fills a line in the line buffer with text content
     */
    void fillLineText(TColor *lineBuffer, uint8_t y, uint8_t *depthBuffer);
public:
    RenderCommand() {};
    RenderCommand* filledRect(TColor color);
    RenderCommand* sprite(const Texture<TColor> *texture);
    RenderCommand* sprite(const Texture<TColor> *texture, uint8_t u, uint8_t v);
    RenderCommand* sprite(const Texture<TColor> *texture, uint8_t u, uint8_t v, bool mirrorh, bool mirrorv);
    RenderCommand* blend(uint8_t blendMode);
    RenderCommand* setDepth(const uint8_t depth);
    void fillLine(TColor *line, uint8_t y, uint8_t *depthBuffer);
};

#define TEMP_TEXTURE_BUFFER_SIZE 8

template<class TColor, int maxCommands>
class RenderBuffer
{
private:
    RenderCommand<TColor> commandList[maxCommands];
    uint8_t commandCount;
    RenderCommand<TColor> noCommand;
    TColor clearColor;
    bool clearBackground;
    // offset to the render commands, useful to ease handling camera positions
    int16_t offsetX, offsetY;
    // applied clipping, useful to limit rendercommands being drawn only in certain areas of the screen
    uint8_t clipTop,clipRight,clipBottom,clipLeft;
    void drawGlyphs(int n, const SpriteGlyph** glyphList, Texture<TColor>* texture, int cursorX, int cursorY,int width, int lineWidth, int hAlign, const uint8_t depth, const uint8_t blendMode);
    Texture<TColor> tempTextureBuffer[TEMP_TEXTURE_BUFFER_SIZE];
    const ImageData* tempTextureBufferImageData[TEMP_TEXTURE_BUFFER_SIZE];
    uint8_t tempTextureBufferUseCount;
    Texture<TColor>* getTempTexture(const ImageData *imageData);
public:
    RenderBuffer() {
        clearBackground = true;
        tempTextureBufferUseCount = 0;
        resetClipping();
        setOffset(0,0);
    };
    int16_t getOffsetX() const {
        return offsetX;
    }
    int16_t getOffsetY() const {
        return offsetY;
    }
    void setOffset(int16_t x, int16_t y) {
        offsetX = x;
        offsetY = y;
    }
    void resetClipping() {
        setClipping(0,RenderBufferConst::screenWidth,RenderBufferConst::screenHeight,0);
    }
    void setClipping(uint8_t top, uint8_t right, uint8_t bottom, uint8_t left) {
        clipTop = top;
        clipRight = right;
        clipBottom = bottom;
        clipLeft = left;
    }
    void setClearBackground(bool clearB) {
        clearBackground = clearB;
    };
    void setClearBackground(bool clearB, TColor color) {
        clearColor = color;
        clearBackground = clearB;
    };
    RenderCommand<TColor>* drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height);
    RenderCommand<TColor>* drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, bool noOffset);
    RenderCommand<TColor>* drawText(const char *text, int16_t x, int16_t y, TColor color, const FONT_INFO *font);
    void drawText(const char* text, int x, int y, int width, int hAlign, const SpriteFont& font, const uint8_t depth);
    void drawText(const char* text, int x, int y, int width, int height, int hAlign, int vAlign, bool wrap, const SpriteFont& font, const uint8_t depth, const uint8_t blendmode);
    void drawText(const char* text, int x, int y, int width, int height, int hAlign, int vAlign, bool wrap, const SpriteFont& font, const uint8_t depth);
    TColor rgb(uint8_t r, uint8_t g, uint8_t b) const;
    bool is16bit() {return sizeof(TColor) == 2;}
    void flush(TinyScreen display);
};

template<class TColor, int maxCommands>
TColor RenderBuffer<TColor,maxCommands>::rgb(uint8_t r, uint8_t g, uint8_t b) const {
    return sizeof(TColor) == 2 ? RGB565(r,g,b) : RGB233(r,g,b);
}

template<class TCol, int maxCommands>
RenderCommand<TCol>* RenderBuffer<TCol, maxCommands>::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    return drawRect(x,y,width,height,false);
}

template<class TCol, int maxCommands>
RenderCommand<TCol>* RenderBuffer<TCol, maxCommands>::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height, bool noOffset)
{
    if (!noOffset) {
        x -= offsetX;
        y -= offsetY;
    }
    if (x >= clipRight || y >= clipBottom || x + width < clipLeft || y + height < clipTop)
        return &noCommand;
    if (commandCount >= maxCommands) return &noCommand;
    RenderCommand<TCol> *cmd = &commandList[commandCount++];
    int16_t right = x + width, bottom = y + height;
    if (x < clipLeft) cmd->rect.x1 = clipLeft, cmd->rect.u = clipLeft-x;
    else       cmd->rect.x1 = x, cmd->rect.u = 0;
    if (y < clipTop) cmd->y1 = clipTop, cmd->rect.v = clipTop-y;
    else       cmd->y1 = y, cmd->rect.v = 0;
    cmd->rect.x2 = right > clipRight ? clipRight : right;
    cmd->y2 = bottom > clipBottom ? clipBottom : bottom;
    cmd->rect.flags = 0;
    return cmd;
}
template<class TCol, int maxCommands>
RenderCommand<TCol>* RenderBuffer<TCol, maxCommands>::drawText(const char *text, int16_t x, int16_t y, TCol color, const FONT_INFO *font)
{
    x -= offsetX;
    y -= offsetY;

    if (y >= clipBottom || y + font->height < clipTop
            || commandCount >= maxCommands) return &noCommand;

    RenderCommand<TCol> *cmd = &commandList[commandCount++];
    cmd->text.x1 = x;
    cmd->y1 = y;
    cmd->y2 = y + font->height;
    cmd->text.color = color;
    cmd->text.font = font;
    cmd->text.text = text;
    cmd->type = RenderCommandType::text;
    return cmd;
}
template<class TColor, int cmdCount>
Texture<TColor>* RenderBuffer<TColor, cmdCount>::getTempTexture(const ImageData *imageData) {
    for (int i=0;i<tempTextureBufferUseCount;i+=1) {
        if (tempTextureBufferImageData[i] == imageData) return &tempTextureBuffer[i];
    }
    if (tempTextureBufferUseCount == TEMP_TEXTURE_BUFFER_SIZE) return 0;
    Texture<TColor> tex(imageData);
    tempTextureBufferImageData[tempTextureBufferUseCount] = imageData;
    tempTextureBuffer[tempTextureBufferUseCount++] = tex;
    return &tempTextureBuffer[tempTextureBufferUseCount-1];
}
template<class TColor, int cmdCount>
void RenderBuffer<TColor, cmdCount>::drawText(const char* text, int x, int y, int width, int hAlign, const SpriteFont& font, uint8_t depth) {
    drawText(text,x,y,width, 0,hAlign,-1,false,font,depth);
}
template<class TColor, int cmdCount>
void RenderBuffer<TColor, cmdCount>::drawText(const char* textSrc, int x, int y, int boxWidth, int boxHeight, int hAlign,int vAlign, bool wrap, const SpriteFont& font, uint8_t depth) {
    drawText(textSrc,x,y,boxWidth,boxHeight,hAlign,vAlign,wrap, font,depth, RenderCommandBlendMode::opaque);
}
template<class TColor, int cmdCount>
void RenderBuffer<TColor, cmdCount>::drawText(const char* textSrc, int x, int y, int boxWidth, int boxHeight, int hAlign,int vAlign, bool wrap, const SpriteFont& font, uint8_t depth, const uint8_t blendMode) {
    if (y - offsetY>clipBottom || y+boxHeight-offsetY < clipTop || x - offsetX> clipRight || x + boxWidth - offsetX < clipLeft) {
        return;
    }
    int len = strlen(textSrc);
    const SpriteGlyph *glyphList[len];
    const char* text = textSrc;

    int cursorX = x;
    int cursorY = y;
    int lineHeight = font.lineHeight;
    Texture<TColor> *texture = getTempTexture(font.imageData);

    int n = 0;
    int height = 0;
    int lineCount = 0;
    int glyphCount = font.glyphCount;
    const SpriteGlyph *glyphs = font.glyphs;
    int lineWidth = 0;
    int lastSpacePos = 0, lastSpaceWidth;
    if (vAlign > -1) {
        height = lineHeight;
        while (char c = *(text++)) {
            if (c <= ' ') lastSpacePos = n;
            for (int i=0;i<glyphCount;i+=1) {
                bool match = glyphs[i].letter == c;

                if (c == '\n' || (match && wrap && n > 0 && lineWidth > boxWidth)) {
                    if (lastSpacePos > 0) {
                        text-=n-lastSpacePos;
                        lastSpacePos = 0;
                    }
                    height +=lineHeight;
                    lineCount+=1;
                    lineWidth = 0;
                    break;
                }
                if (match) {
                    n+=1;
                    lineWidth+=glyphs[i].spacing;
                    break;
                }
            }
        }
        n = 0;
        if (vAlign == 0) cursorY = y + (boxHeight - height) / 2;
        else cursorY = y + boxHeight - height;
        lastSpacePos = 0;
        height = 0;
        lineWidth = 0;
        text = textSrc;
    }
    while (char c = *(text++)) {
        if (c <= ' ') {lastSpacePos = n;lastSpaceWidth = lineWidth;}
        for (int i=0;i<glyphCount;i+=1) {
            bool match = glyphs[i].letter == c;

            if (c == '\n' || (match && wrap && n > 0 && lineWidth > boxWidth)) {
                if (lastSpacePos > 0) {
                    int d = n-lastSpacePos;
                    text-=d;
                    n-=d;
                    lineWidth = lastSpaceWidth;
                    lastSpacePos = 0;
                }
                drawGlyphs(n,glyphList,texture,cursorX, cursorY, boxWidth, lineWidth, hAlign, depth, blendMode);
                n = 0;
                cursorY+=lineHeight;
                lineCount+=1;
                lineWidth = 0;
                break;
            }
            if (match) {
                glyphList[n++] = &glyphs[i];
                lineWidth+=glyphs[i].spacing;
                break;
            }
        }
    }
    drawGlyphs(n,glyphList,texture,cursorX, cursorY, boxWidth, lineWidth, hAlign, depth, blendMode);
 }

template<class TColor, int cmdCount>
void RenderBuffer<TColor, cmdCount>::drawGlyphs(int n, const SpriteGlyph** glyphList, Texture<TColor>* texture, int cursorX, int cursorY, int width, int lineWidth, int hAlign, const uint8_t depth, const uint8_t blendMode) {
    switch (hAlign) {
        case 0: cursorX += (width-lineWidth)/2;break;
        case 1: cursorX += width - lineWidth;break;
    }
    for (int i=0;i<n;i+=1) {
        const SpriteGlyph *g = glyphList[i];
        if (g->w && g->h)
            drawRect(cursorX+g->offsetX, cursorY+g->offsetY,g->w,g->h)->sprite(texture,g->u,g->v)->setDepth(depth)->blend(blendMode);
        cursorX += g->spacing;
    }
}





template<class TCol, int maxCommands>
void RenderBuffer<TCol, maxCommands>::flush(TinyScreen display)
{
    #ifdef DEBUG
    drawText(stringBuffer.start().putDec(commandCount).put("cmd").get(),32,2,rgb(255,255,255), &virtualDJ_5ptFontInfo);
    #endif

    int remainingCount = commandCount;
    RenderCommand<TCol>* remaining[maxCommands];
    RenderCommand<TCol>* active[maxCommands];

    for (int i=0;i<commandCount;i+=1) {
        remaining[i] = &commandList[i];
    }

    display.goTo(0,0);
    #ifdef WIN32
    TCol line[RenderBufferConst::screenWidth + 4];
    unsigned long *check = (unsigned long*)&line[RenderBufferConst::screenWidth];
    *check = 0xbaad;
    #else
    TCol line[RenderBufferConst::screenWidth];
    #endif
    uint8_t depthBuffer[RenderBufferConst::screenWidth];
    display.startData();

    const uint8_t stepSize = 8;

    for (uint8_t yg=0; yg<RenderBufferConst::screenHeight; yg+=stepSize)
    {
        uint8_t yLimit = yg + stepSize;
        uint8_t newRemainingCount = 0;
        uint8_t activeCount = 0;
        for (uint8_t i = 0; i < remainingCount; i+=1) {
            RenderCommand<TCol>* rc = remaining[i];
            if (rc->y2 < yg) {
                // command fallen out
                continue;
            }
            remaining[newRemainingCount++] = rc;
            if (rc->y1 <= yLimit)
            {
                active[activeCount++] = rc;
            }
        }
        remainingCount = newRemainingCount;
        for (uint8_t y=yg;y < yLimit; y += 1) {
            if (sizeof(TCol) == 1) {
                if (clearBackground) {
                    memset(line,clearColor,RenderBufferConst::screenWidth * sizeof(TCol));

                }

            } else {
                if (clearBackground) {
                    for (uint8_t x=0;x<RenderBufferConst::screenWidth;x+=1)
                        line[x] = clearColor;
                }
            }
            memset(depthBuffer,0,RenderBufferConst::screenWidth);
            int newActiveCount = 0;
            for (uint8_t i=0; i<activeCount; i+=1)
            {
                RenderCommand<TCol>* rc = active[i];
                if (rc->y2 < y) {
                    continue;
                }
                active[i]->fillLine(line, y, depthBuffer);
                active[newActiveCount++] = rc;
            }
            #ifdef WIN32
            assert(*check == 0xbaad);
            #endif // WIN32
            activeCount = newActiveCount;
            //line[remainingCount] = rgb(255,0,0);
            //line[activeCount] = rgb(0,255,0);
            display.writeBuffer((uint8_t*)line, RenderBufferConst::screenWidth * sizeof(TCol));
        }
    }
    display.endTransfer();
    commandCount = 0;
    tempTextureBufferUseCount = 0;
}

#endif
