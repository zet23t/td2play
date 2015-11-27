#ifndef __RENDERBUFFER_H__
#define __RENDERBUFFER_H__

#include <assert.h>

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
}

namespace TextureType {
    const uint8_t rgb565sram = 1;
    const uint8_t rgb565progmem = 2;
}

// Creates a RGB-565 encoded two byte sequence. The encoding is more difficult than I thought it would be due
// to endian encoding. I don't know how to write this in a better way.
#define RGB565(r,g,b) (uint16_t)(((unsigned char)(r) >> 3) << 8 | ((unsigned char)(g) >> 5 | ((unsigned char)(g) >> 3 & 31) << 13) | ((unsigned char)(b) >> 3 << 3))
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
        uint8_t *data;
        uint16_t *rgb565;
    };
    uint8_t type;
    uint16_t width;
    uint16_t height;
    uint16_t widthMod;
    uint16_t heightMod;
    uint16_t transparentColorMask;

    void fillLineRgb565sram (TColor *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v, uint8_t width, uint8_t blendMode) const;
public:
    Texture (uint8_t *data, uint8_t type, uint16_t width, uint16_t height, uint16_t transparentColorMask);
    void fillLine(TColor *lineBuffer, uint8_t lineX, uint8_t u, uint8_t v, uint8_t width, uint8_t blendMode) const;
};

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
    uint8_t type;
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
    void fillLineText(TColor *lineBuffer, uint8_t y);
public:
    RenderCommand() {};
    RenderCommand* filledRect(TColor color);
    RenderCommand* sprite(const Texture<TColor> *texture);
    RenderCommand* blend(uint8_t blendMode);
    void fillLine(TColor *line, uint8_t y);
};


template<class TColor, int maxCommands>
class RenderBuffer
{
private:
    RenderCommand<TColor> commandList[maxCommands];
    uint8_t commandCount;
    RenderCommand<TColor> noCommand;
public:
    RenderBuffer() {};
    RenderCommand<TColor>* drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height);
    RenderCommand<TColor>* drawText(const char *text, int16_t x, int16_t y, TColor color, const FONT_INFO *font);
    TColor rgb(uint8_t r, uint8_t g, uint8_t b) const;
    void flush(TinyScreen display);
};

template<class TColor, int maxCommands>
TColor RenderBuffer<TColor,maxCommands>::rgb(uint8_t r, uint8_t g, uint8_t b) const {
    return sizeof(TColor) == 2 ? RGB565(r,g,b) : RGB233(r,g,b);
}

template<class TCol, int maxCommands>
RenderCommand<TCol>* RenderBuffer<TCol, maxCommands>::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    if (x >= RenderBufferConst::screenWidth || y >= RenderBufferConst::screenHeight || x + width < 0 || y + height < 0)
        return &noCommand;
    if (commandCount >= maxCommands) return &noCommand;
    RenderCommand<TCol> *cmd = &commandList[commandCount++];
    int16_t right = x + width, bottom = y + height;
    if (x < 0) cmd->rect.x1 = 0, cmd->rect.u = -x;
    else       cmd->rect.x1 = x, cmd->rect.u = 0;
    if (y < 0) cmd->y1 = 0, cmd->rect.v = -y;
    else       cmd->y1 = y, cmd->rect.v = 0;
    cmd->rect.x2 = right > RenderBufferConst::screenWidth ? RenderBufferConst::screenWidth : right;
    cmd->y2 = bottom > RenderBufferConst::screenHeight ? RenderBufferConst::screenHeight : bottom;
    return cmd;
}

template<class TCol, int maxCommands>
RenderCommand<TCol>* RenderBuffer<TCol, maxCommands>::drawText(const char *text, int16_t x, int16_t y, TCol color, const FONT_INFO *font)
{
    if (y >= RenderBufferConst::screenHeight || y + font->height < 0
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

template<class TCol, int maxCommands>
void RenderBuffer<TCol, maxCommands>::flush(TinyScreen display)
{
    display.goTo(0,0);
    TCol line[RenderBufferConst::screenWidth];
    display.startData();
    for (uint8_t y=0; y<RenderBufferConst::screenHeight; y+=1)
    {
        memset(line,0,sizeof(line));
        for (uint8_t i=0; i<commandCount; i+=1)
        {
            commandList[i].fillLine(line, y);
        }
        display.writeBuffer((uint8_t*)line, sizeof(line));
    }
    display.endTransfer();
    commandCount = 0;
}

#endif
