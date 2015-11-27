#include <stdint.h>
#include <TinyScreen.h>
#include "lib_RenderBuffer.h"

template<class TColor>
void Texture<TColor>::fillLineRgb565sram (TColor *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v, uint8_t width, uint8_t blendMode) const  {
    int offset = (v & heightMod) * this->width;
    int pos = u;
    if (transparentColorMask) {
        switch(blendMode) {
        case RenderCommandBlendMode::opaque:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t col = rgb565[index];
                if (col != transparentColorMask) {
                    lineBuffer[lineX] = col;
                }
                lineX+=1;
            }
            break;
        case RenderCommandBlendMode::bitwiseOr:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t col = rgb565[index];
                if (col != transparentColorMask) {
                    lineBuffer[lineX] |= col;
                }
                lineX+=1;
            }
            break;
        case RenderCommandBlendMode::bitwiseAnd:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t col = rgb565[index];
                if (col != transparentColorMask) {
                    lineBuffer[lineX] &= col;
                }
                lineX+=1;
            }
            break;
        case RenderCommandBlendMode::average:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t col = rgb565[index];
                if (col != transparentColorMask) {
                    uint16_t dst = lineBuffer[lineX];
                    col = col & ~(RGB565(1,1,1)) >> 1;
                    dst = dst & ~(RGB565(1,1,1)) >> 1;
                    lineBuffer[lineX] = col + dst;
                }
                lineX+=1;
            }
            break;
        }

    } else {
        switch(blendMode) {
        case RenderCommandBlendMode::opaque:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                lineBuffer[lineX++] = rgb565[index];
            }
            break;
        case RenderCommandBlendMode::bitwiseOr:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                lineBuffer[lineX++] |= rgb565[index];
            }
            break;
        case RenderCommandBlendMode::bitwiseAnd:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                lineBuffer[lineX++] &= rgb565[index];
            }
            break;
        case RenderCommandBlendMode::average:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t dst = lineBuffer[lineX] & ~(RGB565(1,1,1)) >> 1;
                uint16_t col = rgb565[index] & ~(RGB565(1,1,1)) >> 1;
                lineBuffer[lineX++] = col + dst;
            }
            break;
        }
    }
}

template<class TColor>
Texture<TColor>::Texture (uint8_t *data, uint8_t type, uint16_t width, uint16_t height, uint16_t transparentColorMask) {
    this->data = data;
    this->type = type;
    this->width = width;
    this->height = height;
    this->widthMod = width - 1;
    this->heightMod = height - 1;
    this->transparentColorMask = transparentColorMask;
    assert((this->width & this->widthMod) == 0);
    assert((this->height & this->heightMod) == 0);
}

template<class TColor>
void Texture<TColor>::fillLine(TColor *lineBuffer, uint8_t lineX, uint8_t u, uint8_t v, uint8_t width, uint8_t blendMode) const {
    switch (type) {
    case TextureType::rgb565sram: fillLineRgb565sram(lineBuffer,lineX,u,v,width, blendMode); break;
    }
}

/**
 * Fills a line in the line buffer with text content
 */
template<class TColor>
void RenderCommand<TColor>::fillLineText(TColor *lineBuffer, uint8_t y)
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

template<class TColor>
RenderCommand<TColor>* RenderCommand<TColor>::filledRect(TColor color)
{
    this->rect.color = color;
    this->rect.blendMode = RenderCommandBlendMode::opaque;
    this->type = RenderCommandType::solid;
    return this;
}

template<class TColor>
RenderCommand<TColor>* RenderCommand<TColor>::sprite(const Texture<TColor> *texture)
{
    this->rect.texture = texture;
    this->type = RenderCommandType::textured;
    return this;
}

template<class TColor>
RenderCommand<TColor>* RenderCommand<TColor>::blend(uint8_t blendMode) {
    this->rect.blendMode = blendMode;
    return this;
}

template<class TColor>
void RenderCommand<TColor>::fillLine(TColor *line, uint8_t y)
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

template<class TCol>
RenderCommand<TCol>* RenderBuffer<TCol>::drawRect(int16_t x, int16_t y, uint16_t width, uint16_t height)
{
    if (x >= RenderBufferConst::screenWidth || y >= RenderBufferConst::screenHeight || x + width < 0 || y + height < 0)
        return &noCommand;
    if (commandCount >= RenderBufferConst::maxCommands) return &noCommand;
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

template<class TCol>
RenderCommand<TCol>* RenderBuffer<TCol>::drawText(const char *text, int16_t x, int16_t y, TCol color, const FONT_INFO *font)
{
    if (y >= RenderBufferConst::screenHeight || y + font->height < 0
            || commandCount >= RenderBufferConst::maxCommands) return &noCommand;
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

template<class TCol>
void RenderBuffer<TCol>::flush(TinyScreen display)
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

// make sure we generate all function with uint16 / uint8 signture
template class Texture<uint16_t>;
template class RenderCommand<uint16_t>;
template class RenderBuffer<uint16_t>;

template class Texture<uint8_t>;
template class RenderCommand<uint8_t>;
template class RenderBuffer<uint8_t>;
