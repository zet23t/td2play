#ifndef __RENDERBUFFER_H__
#define __RENDERBUFFER_H__

#include <assert.h>

namespace RenderBufferConst {
    const uint8_t maxCommands = 48;
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

#ifdef WIN32
#include <memory.h>
#define pgm_read_byte *
#define pgm_read_word *

#endif

#ifndef PROGMEM
#define PROGMEM
#endif

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

    void fillLineRgb565sram (uint16_t *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v, uint8_t width, uint8_t blendMode) const;
public:
    Texture (uint8_t *data, uint8_t type, uint16_t width, uint16_t height, uint16_t transparentColorMask);
    void fillLine(uint16_t *lineBuffer, uint8_t lineX, uint8_t u, uint8_t v, uint8_t width, uint8_t blendMode) const;
};

namespace RenderCommandData {
    struct Rect {
        union {
            /**
             * RGB-565 color for the command
             */
            uint16_t color;
            const Texture *texture;
        };
        uint8_t x1, x2;
        uint8_t u, v;
        uint8_t blendMode;
    };
    struct Text {
        /**
         * RGB-565 color for the command
         */
        uint16_t color;
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
        RenderCommandData::Rect rect;
        RenderCommandData::Text text;
    };

private:
    /**
     * Fills a line in the line buffer with text content
     */
    void fillLineText(uint16_t *lineBuffer, uint8_t y);
public:
    RenderCommand() {};
    RenderCommand* filledRect(uint16_t color);
    RenderCommand* sprite(const Texture *texture);
    RenderCommand* blend(uint8_t blendMode);
    void fillLine(uint16_t *line, uint8_t y);
};


class RenderBuffer
{
private:
    RenderCommand commandList[RenderBufferConst::maxCommands];
    uint8_t commandCount;
    RenderCommand noCommand;
public:
    RenderBuffer() {};
    RenderCommand* drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height);
    RenderCommand* drawText(const char *text, int16_t x, int16_t y, uint16_t color, const FONT_INFO *font);
    void flush(TinyScreen display);
};

#endif
