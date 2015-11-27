#ifndef __RENDERBUFFER_H__
#define __RENDERBUFFER_H__

#include <assert.h>

namespace RenderBufferConst {
    const uint8_t maxCommands = 128;
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
    void fillLineText(uint16_t *lineBuffer, uint8_t y)
    {
        uint8_t fontHeight = text.font->height;
        //if(y >= fontY && y < fontY + fontHeight) // checked in caller already
        {
            const FONT_CHAR_INFO* fontDescriptor = text.font->charDesc;
            const unsigned char* fontBitmap = text.font->bitmap;
            uint8_t fontFirstCh = text.font->startCh;
            //uint8_t fontLastCh = font->endCh;
            uint8_t stringChar = 0;
            int16_t fontX = text.x1;
            int8_t fontY = y1;
            uint8_t ch = text.text[stringChar++];
            while(ch)
            {
                uint8_t chWidth = pgm_read_byte(&fontDescriptor[ch - fontFirstCh].width);
                uint8_t bytesPerRow = chWidth / 8;
                if(chWidth > bytesPerRow * 8)
                    bytesPerRow++;
                uint16_t offset = pgm_read_word(&fontDescriptor[ch - fontFirstCh].offset) + (bytesPerRow * fontHeight) - 1;
                const unsigned char *coffset = offset + fontBitmap - (y - fontY);
                for(uint8_t byte = 0; byte < bytesPerRow; byte++)
                {
                    uint8_t data = pgm_read_byte(coffset - ((bytesPerRow - byte - 1) * fontHeight));
                    uint8_t bits = byte * 8;
                    for(uint8_t i = 0; i < 8 && (bits + i) < chWidth && fontX < 96; ++i)
                    {
                        if((data & (0x80 >> i)) && fontX >= 0)
                        {
                            lineBuffer[fontX] = text.color;
                        }
                        ++fontX;
                    }
                }
                fontX += 1;
                ch = text.text[stringChar++];
            }
        }
    }
public:
    RenderCommand() {};
    RenderCommand* filledRect(uint16_t color)
    {
        this->rect.color = color;
        this->rect.blendMode = RenderCommandBlendMode::opaque;
        this->type = RenderCommandType::solid;
        return this;
    }

    RenderCommand* sprite(const Texture *texture)
    {
        this->rect.texture = texture;
        this->type = RenderCommandType::textured;
        return this;
    }

    RenderCommand* blend(uint8_t blendMode) {
        this->rect.blendMode = blendMode;
        return this;
    }

    void fillLine(uint16_t *line, uint8_t y)
    {
        if (y>=y1 && y < y2)
        {
            switch(type)
            {
            case RenderCommandType::solid:
                for (uint8_t x = rect.x1; x < rect.x2; x+=1) line[x] = rect.color;
                break;
            case RenderCommandType::textured:
                rect.texture->fillLine(line, rect.x1, rect.u, rect.v + y - y1, rect.x2 - rect.x1, rect.blendMode);
                break;
            case RenderCommandType::text:
                fillLineText(line, y);
                break;
            }
        }
    }
};


class RenderBuffer
{
private:
    RenderCommand commandList[RenderBufferConst::maxCommands];
    uint8_t commandCount;
    RenderCommand noCommand;
public:
    RenderBuffer() {};
    RenderCommand* drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height)
    {
        if (x >= RenderBufferConst::screenWidth || y >= RenderBufferConst::screenHeight || x + width < 0 || y + height < 0)
            return &noCommand;
        if (commandCount >= RenderBufferConst::maxCommands) return &noCommand;
        RenderCommand *cmd = &commandList[commandCount++];
        int16_t right = x + width, bottom = y + height;
        if (x < 0) cmd->rect.x1 = 0, cmd->rect.u = -x;
        else       cmd->rect.x1 = x, cmd->rect.u = 0;
        if (y < 0) cmd->y1 = 0, cmd->rect.v = -y;
        else       cmd->y1 = y, cmd->rect.v = 0;
        cmd->rect.x2 = right > RenderBufferConst::screenWidth ? RenderBufferConst::screenWidth : right;
        cmd->y2 = bottom > RenderBufferConst::screenHeight ? RenderBufferConst::screenHeight : bottom;
        return cmd;
    }
    RenderCommand* drawText(const char *text, int16_t x, int16_t y, uint16_t color, const FONT_INFO *font)
    {
        if (y >= RenderBufferConst::screenHeight || y + font->height < 0
                || commandCount >= RenderBufferConst::maxCommands) return &noCommand;
        RenderCommand *cmd = &commandList[commandCount++];
        cmd->text.x1 = x;
        cmd->y1 = y;
        cmd->y2 = y + font->height;
        cmd->text.color = color;
        cmd->text.font = font;
        cmd->text.text = text;
        cmd->type = RenderCommandType::text;
        return cmd;
    }
    void flush(TinyScreen display)
    {
        display.goTo(0,0);
        uint16_t line[RenderBufferConst::screenWidth];
        display.startData();
        for (uint8_t y=0; y<RenderBufferConst::screenHeight; y+=1)
        {
            memset(line,0,sizeof(line));
            for (uint8_t i=0; i<commandCount; i+=1)
            {
                commandList[i].fillLine(line, y);
            }
            display.writeBuffer((uint8_t*)line, RenderBufferConst::screenWidth * 2);
        }
        display.endTransfer();
        commandCount = 0;
    }
};

#endif
