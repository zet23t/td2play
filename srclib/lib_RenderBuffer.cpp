#include <stdint.h>
#include "lib_RenderBuffer.h"
#ifndef WIN32
#include <avr/pgmspace.h>
#else
#define pgm_read_word_far(x) *(x)
#define pgm_read_byte_far(x) (*(x) & 0xff)
#endif // WIN32

static uint16_t read_word(const uint16_t* ptr) {
    return pgm_read_byte_far(ptr) | pgm_read_byte_far(((uint8_t*)ptr)+1)<<8;
    //return pgm_read_word_far(ptr);
}

template<class TColor>
void Texture<TColor>::fillLineRgb565(bool sram, TColor *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v, uint8_t width, uint8_t blendMode, uint8_t *depthBuffer, uint8_t depth) const  {
    int offset = (v & heightMod) << widthBits;
    int pos = u;
    if (transparentColorMask) {
        switch(blendMode) {
        case RenderCommandBlendMode::opaque:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                if (depth < depthBuffer[lineX]) {
                    pos+=1;
                    lineX+=1;
                    continue;
                }
                int index = (pos++ & widthMod) + offset;
                uint16_t col = sram ? rgb565[index] : read_word(&rgb565[index]);
                if (col != transparentColorMask) {
                    depthBuffer[lineX] = depth;
                    if (sizeof(TColor) == 2) {
                        lineBuffer[lineX] = col;
                    } else {
                        uint8_t r = col & 31;
                        uint8_t g = col >> 5 & 63;
                        uint8_t b = col >> 11 & 31;

                        lineBuffer[lineX] = (r >> 3) | (g & 034) | (b << 4 & 0160);
                    }
                }

                lineX+=1;
            }
            break;
        case RenderCommandBlendMode::add:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t col = sram ? rgb565[index] : read_word(&rgb565[index]);
                if (col > 0 && col != transparentColorMask && depth >= depthBuffer[lineX]) {
                    depthBuffer[lineX] = depth;
                    int dst = lineBuffer[lineX];
                    int dstr = RGB565_TO_RED(dst) + RGB565_TO_RED(col);
                    int dstg = RGB565_TO_GREEN(dst) + RGB565_TO_GREEN(col);
                    int dstb = RGB565_TO_BLUE(dst) + RGB565_TO_BLUE(col);
                    if (dstr > 31) dstr = 31;
                    if (dstg > 63) dstg = 63;
                    if (dstb > 31) dstb = 31;

                    lineBuffer[lineX] = RGB565RAW(dstr,dstg,dstb);
                }
                lineX+=1;
            }
            break;
        case RenderCommandBlendMode::bitwiseOr:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t col = sram ? rgb565[index] : read_word(&rgb565[index]);
                if (col != transparentColorMask && col > 0 && depth >= depthBuffer[lineX]) {
                    depthBuffer[lineX] = depth;
                    lineBuffer[lineX] |= col;
                }
                lineX+=1;
            }
            break;
        case RenderCommandBlendMode::bitwiseAnd:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t col = sram ? rgb565[index] : read_word(&rgb565[index]);
                if (col != transparentColorMask && depth >= depthBuffer[lineX]) {
                    depthBuffer[lineX] = depth;
                    lineBuffer[lineX] &= col;
                }
                lineX+=1;
            }
            break;
        case RenderCommandBlendMode::subtract:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t src = (sram ? rgb565[index] : read_word(&rgb565[index]));
                if (src == transparentColorMask || depth < depthBuffer[lineX]) {
                    lineX+=1;
                    continue;
                }
                depthBuffer[lineX] = depth;

                uint16_t dst = lineBuffer[lineX];
                uint8_t dstR = RGB565_TO_RED(dst);
                uint8_t dstG = RGB565_TO_GREEN(dst);
                uint8_t dstB = RGB565_TO_BLUE(dst);
                uint8_t srcR = RGB565_TO_RED(src);
                uint8_t srcG = RGB565_TO_GREEN(src);
                uint8_t srcB = RGB565_TO_BLUE(src);
                dstR = dstR > srcR ? dstR - srcR : 0;
                dstG = dstG > srcG ? dstG - srcG : 0;
                dstB = dstB > srcB ? dstB - srcB : 0;
                uint16_t result = RGB565(dstR,dstG,dstB);
                if (sizeof(TColor) == 2) {
                    lineBuffer[lineX++] = result;
                } else {
                    uint8_t col = result >> 8 | result << 8;
                    uint8_t r = (col & 31);
                    uint8_t g = (col >> 5 & 63);
                    uint8_t b = (col >> 11 & 31);

                    lineBuffer[lineX++] = (r >> 3) | (g & 034) | (b << 3 & 0340);
                }
            }
            break;

        case RenderCommandBlendMode::average:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t src = sram ? rgb565[index] : read_word(&rgb565[index]);
                if (src == transparentColorMask || depth < depthBuffer[lineX]) {
                    lineX+=1;
                    continue;
                }
                depthBuffer[lineX] = depth;
                if (sizeof(TColor) == 2) {
                    uint16_t dstR = RGB565_TO_RED(lineBuffer[lineX]);
                    uint16_t dstG = RGB565_TO_GREEN(lineBuffer[lineX]);
                    uint16_t dstB = RGB565_TO_BLUE(lineBuffer[lineX]);

                    uint16_t srcR = RGB565_TO_RED(src);
                    uint16_t srcG = RGB565_TO_GREEN(src);
                    uint16_t srcB = RGB565_TO_BLUE(src);

                    uint8_t r = (srcR >> 1) + (dstR >> 1);
                    uint8_t g = (srcG >> 1) + (dstG >> 1);
                    uint8_t b = (srcB >> 1) + (dstB >> 1);
                    uint16_t result = RGB565(r,g,b);
                    lineBuffer[lineX++] = result;
                } else {
                    uint16_t col = src >> 8 | src << 8;
                    uint8_t r = (col & 31);
                    uint8_t g = (col >> 5 & 63);
                    uint8_t b = (col >> 11 & 31);
                    col = (r >> 3) | (g & 034) | (b << 3 & 0340);
                    uint16_t dst = lineBuffer[lineX] & ~(RGB233(1,1,1)) >> 1;
                    lineBuffer[lineX++] = ((col & ~(RGB233(1,1,1))) >> 1) + dst;
                }


            }
            break;
        }

    } else {
        switch(blendMode) {
        case RenderCommandBlendMode::opaque:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t src = sram ? rgb565[index] : read_word(&rgb565[index]);
                if (sizeof(TColor) == 2) {
                    if (depth >= depthBuffer[lineX]) {
                        depthBuffer[lineX] = depth;
                        lineBuffer[lineX++] = src;
                    }
                } else {
                    uint16_t col = src << 8 | src >> 8;//data[index*2+1] | (data[index*2] << 8);
                    uint8_t r = (col & 31);
                    uint8_t g = (col >> 5 & 63);
                    uint8_t b = (col >> 11 & 31);

                    if (depth >= depthBuffer[lineX]) {
                        depthBuffer[lineX] = depth;
                        lineBuffer[lineX++] = (r >> 3) | (g & 034) | (b << 3 & 0340);
                    }
                }
            }
            break;
        case RenderCommandBlendMode::bitwiseOr:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                lineBuffer[lineX++] |= sram ? rgb565[index] : read_word(&rgb565[index]);
            }
            break;
        case RenderCommandBlendMode::bitwiseAnd:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                lineBuffer[lineX++] &= sram ? rgb565[index] : read_word(&rgb565[index]);
            }
            break;
        case RenderCommandBlendMode::add:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t col = sram ? rgb565[index] : read_word(&rgb565[index]);
                if (col > 0 && depth >= depthBuffer[lineX]) {
                    depthBuffer[lineX] = depth;
                    int dst = lineBuffer[lineX];
                    int dstr = RGB565_TO_RED(dst) + RGB565_TO_RED(col);
                    int dstg = RGB565_TO_GREEN(dst) + RGB565_TO_GREEN(col);
                    int dstb = RGB565_TO_BLUE(dst) + RGB565_TO_BLUE(col);
                    if (dstr > 31) dstr = 31;
                    if (dstg > 63) dstg = 63;
                    if (dstb > 31) dstb = 31;

                    lineBuffer[lineX] = RGB565RAW(dstr,dstg,dstb);
                }
                lineX+=1;
            }
            break;
        case RenderCommandBlendMode::subtract:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t dst = lineBuffer[lineX];
                uint8_t dstR = RGB565_TO_RED(dst);
                uint8_t dstG = RGB565_TO_GREEN(dst);
                uint8_t dstB = RGB565_TO_BLUE(dst);
                uint16_t src = (sram ? rgb565[index] : read_word(&rgb565[index]));
                uint8_t srcR = RGB565_TO_RED(src);
                uint8_t srcG = RGB565_TO_GREEN(src);
                uint8_t srcB = RGB565_TO_BLUE(src);
                dstR = dstR > srcR ? dstR - srcR : 0;
                dstG = dstG > srcG ? dstG - srcG : 0;
                dstB = dstB > srcB ? dstB - srcB : 0;
                uint16_t result = RGB565(dstR,dstG,dstB);
                if (sizeof(TColor) == 2) {
                    lineBuffer[lineX++] = dst;
                } else {
                    uint8_t col = result >> 8 | result << 8;
                    uint8_t r = (col & 31);
                    uint8_t g = (col >> 5 & 63);
                    uint8_t b = (col >> 11 & 31);

                    lineBuffer[lineX++] = (r >> 3) | (g & 034) | (b << 3 & 0340);
                }
            }
            break;
        case RenderCommandBlendMode::average:
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                int index = (pos++ & widthMod) + offset;
                uint16_t dst = lineBuffer[lineX] & ~(RGB565(1,1,1)) >> 1;
                uint16_t col = (sram ? rgb565[index] : read_word(&rgb565[index])) & ~(RGB565(1,1,1)) >> 1;
                uint16_t result = col + dst;
                if (sizeof(TColor) == 2) {
                    lineBuffer[lineX++] = result;
                } else {
                    col = result >> 8 | result << 8;
                    uint8_t r = (col & 31);
                    uint8_t g = (col >> 5 & 63);
                    uint8_t b = (col >> 11 & 31);

                    lineBuffer[lineX++] = (r >> 3) | (g & 034) | (b << 3 & 0340);
                }
            }
            break;
        default:
#ifdef WIN32
    assert(0);
#endif // WIN32
            for (uint8_t x = lineX; x < lineX+width; x+=1) lineBuffer[x] = 0xff;
            break;
        }
    }
}

template<class TColor>
bool Texture<TColor>::isTransparent(uint16_t x, uint16_t y) const {
    if (transparentColorMask == 0) return false;
    return getColor(x,y) == transparentColorMask;
}

template<class TColor>
TColor Texture<TColor>::getColor(uint16_t x, uint16_t y) const {
    if (transparentColorMask == 0) return 0;

    x &= widthMod;
    y &= heightMod;

    int index = x + (y << widthBits);
    const bool sram = (type == TextureType::rgb233sram || type == TextureType::rgb565sram);
    TColor col = sizeof(TColor) == 2 ?
        sram ? rgb565[index] : read_word(&rgb565[index])
        :
        sram ? rgb233[index] : pgm_read_byte_far(&rgb233[index]);

    return col;
}

template<class TColor>
void Texture<TColor>::fillLineRgb233sram (TColor *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v, uint8_t width, uint8_t blendMode, uint8_t *depthBuffer, uint8_t depth) const  {
    int offset = (v & heightMod) * this->width;
    const uint8_t *rgb233Line = &rgb233[offset];

    int pos = u;
    if (transparentColorMask) {
        if (sizeof(TColor) == 1) {
            const uint8_t* rgb233LineEnd = rgb233Line + this->width;
            const uint8_t* rgb233LineStart = rgb233Line;
            for (uint8_t i = 0; i < width && lineX < RenderBufferConst::screenWidth; i+=1)
            {
                //uint8_t rgb = rgb233Line[pos++ & widthMod];
                uint8_t rgb = *rgb233Line;
                rgb233Line+=1;
                if (rgb233Line == rgb233LineEnd) rgb233Line = rgb233LineStart;
                if (transparentColorMask != rgb)
                    lineBuffer[lineX] = rgb;
                lineX+=1;
            }
        } else {

        }
    } else {
        if (sizeof(TColor) == 1) {
            int index = (pos & widthMod);
            int rest = this->width - index;
            int x2 = lineX + width;
            if (x2 > RenderBufferConst::screenWidth) x2 = RenderBufferConst::screenWidth;
            int sz = width - lineX;
            if (rest >= sz) {
                memcpy(&lineBuffer[lineX],&rgb233Line[index], sz);
                return;
            }
            memcpy(&lineBuffer[lineX], &rgb233Line[index], rest);
            // first part copied, restart at 0
            // Minimize repeated copies of small sources by using the line buffer itself
            // By copying the already filled linebuffer, we can double the memcopy block with
            // each iteration.
            // So if our source image is 4 pixels wide and have to fill 96 pixels, the
            // memcpy blocksize is 4+4+8+16+32+ 32(rest) - which is less than the otherwise
            // 24 memcpy calls. Though there's more logic, it is in deed faster this way for
            // many cases, especially when the source texture is very narrow. Memcpy is very fast.
            lineX+=rest;
            const uint8_t* src = rgb233Line;
            const uint8_t* cpStarted = (uint8_t*)&lineBuffer[lineX];
            uint8_t cpWidth = this->width;
            while (lineX < x2) {
                rest = x2 - lineX;
                if (rest > cpWidth) rest = cpWidth;
                memcpy(&lineBuffer[lineX], src, rest);
                lineX+=rest;
                if (src == cpStarted) {
                    cpWidth *= 2;
                } else {
                    src = cpStarted;
                }
            }
        } else {

        }
    }
}

template<class TColor>
void Texture<TColor>::fillLineRgb233progmem (TColor *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v, uint8_t width, uint8_t blendMode, uint8_t* depthBuffer, uint8_t depth) const  {
}
template<class TColor>
void Texture<TColor>::init (const uint8_t *data, uint8_t type, uint16_t width, uint16_t height, uint16_t transparentColorMask) {
    this->data = data;
    this->type = type;
    this->width = width;
    this->height = height;
    this->widthMod = width - 1;
    this->heightMod = height - 1;
    widthBits = 0;
    this->transparentColorMask = transparentColorMask;
    assert((this->width & this->widthMod) == 0);
    assert((this->height & this->heightMod) == 0);
    while ((1<<widthBits) & widthMod) widthBits+=1;
}

template<class TColor>
Texture<TColor>::Texture (const uint8_t *data, uint8_t type, uint16_t width, uint16_t height, uint16_t transparentColorMask) {
    init(data,type,width,height,transparentColorMask);
}

template<class TColor>
Texture<TColor>::Texture (const ImageData& img) {
    init(img.data, img.format, img.width, img.height, (TColor)img.transparentColor);
}

template<class TColor>
Texture<TColor>::Texture (const ImageData* img) {
    init(img->data, img->format, img->width, img->height, (TColor)img->transparentColor);
}


template<class TColor>
void Texture<TColor>::fillLine(TColor *lineBuffer, uint8_t lineX, uint8_t u, uint8_t v, uint8_t width, uint8_t blendMode, uint8_t *depthBuffer, uint8_t depth) const {
    switch (type) {
    case TextureType::rgb565sram: fillLineRgb565(true, lineBuffer,lineX,u,v,width, blendMode,depthBuffer,depth); break;
    case TextureType::rgb565progmem: fillLineRgb565(false, lineBuffer,lineX,u,v,width, blendMode,depthBuffer,depth); break;
    case TextureType::rgb233sram: fillLineRgb233sram(lineBuffer,lineX,u,v,width, blendMode,depthBuffer,depth); break;
    case TextureType::rgb233progmem: fillLineRgb233progmem(lineBuffer,lineX,u,v,width, blendMode,depthBuffer,depth); break;
    }
}

/**
 * Fills a line in the line buffer with text content
 */
template<class TColor>
void RenderCommand<TColor>::fillLineText(TColor *lineBuffer, uint8_t y,uint8_t *depthBuffer)
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
            uint16_t offset = read_word(&fontDescriptor[ch - fontFirstCh].offset) + (bytesPerRow * fontHeight) - 1;
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
    this->depth = 0;
    return this;
}

template<class TColor>
RenderCommand<TColor>* RenderCommand<TColor>::sprite(const Texture<TColor> *texture)
{
    return sprite(texture,0,0);
}

template<class TColor>
RenderCommand<TColor>* RenderCommand<TColor>::sprite(const Texture<TColor> *texture, uint8_t u, uint8_t v)
{
    #ifdef WIN32
    if (!texture) {
        assert(texture);
    }
    #endif // WIN32
    this->rect.texture = texture;
    this->rect.u += u;
    this->rect.v += v;
    this->type = RenderCommandType::textured;
    this->rect.blendMode = RenderCommandBlendMode::opaque;
    this->depth = 0;
    return this;
}

template<class TColor>
RenderCommand<TColor>* RenderCommand<TColor>::blend(uint8_t blendMode) {
    this->rect.blendMode = blendMode;
    return this;
}

template<class TColor>
RenderCommand<TColor>* RenderCommand<TColor>::setDepth(const uint8_t depth) {
    this->depth = depth;
    return this;
}

template<class TColor>
void RenderCommand<TColor>::fillLine(TColor *line, uint8_t y, uint8_t *depthBuffer)
{
    if (y>=y1 && y < y2)
    {
        switch(type)
        {
        case RenderCommandType::solid:
            switch(rect.blendMode) {
            default:
                for (uint8_t x = rect.x1; x < rect.x2; x+=1) {
                    if (depthBuffer[x] <= depth) {
                        line[x] = rect.color;
                        depthBuffer[x] = depth;
                    }
                }
                break;
            case RenderCommandBlendMode::bitwiseAnd:
                for (uint8_t x = rect.x1; x < rect.x2; x+=1) {
                    if (depthBuffer[x] <= depth) {
                        line[x] &= rect.color;
                        depthBuffer[x] = depth;
                    }
                }
                break;
            case RenderCommandBlendMode::bitwiseOr:
                for (uint8_t x = rect.x1; x < rect.x2; x+=1) {
                    if (depthBuffer[x] <= depth) {
                        line[x] |= rect.color;
                        depthBuffer[x] = depth;
                    }
                }
                break;
            case RenderCommandBlendMode::subtract:
                {
                    uint16_t src = rect.color;
                    for (uint8_t x = rect.x1; x < rect.x2; x+=1) {
                        if (depthBuffer[x] <= depth) {
                            uint16_t dst = line[x];
                            uint8_t dstR = RGB565_TO_RED(dst);
                            uint8_t dstG = RGB565_TO_GREEN(dst);
                            uint8_t dstB = RGB565_TO_BLUE(dst);
                            uint8_t srcR = RGB565_TO_RED(src);
                            uint8_t srcG = RGB565_TO_GREEN(src);
                            uint8_t srcB = RGB565_TO_BLUE(src);
                            dstR = dstR > srcR ? dstR - srcR : 0;
                            dstG = dstG > srcG ? dstG - srcG : 0;
                            dstB = dstB > srcB ? dstB - srcB : 0;
                            uint16_t result = RGB565RAW(dstR,dstG,dstB);
                            line[x] = result;
                            depthBuffer[x] = depth;
                        }
                    }
                }
                break;
            case RenderCommandBlendMode::add:
                {
                    uint16_t src = rect.color;
                    for (uint8_t x = rect.x1; x < rect.x2; x+=1) {
                        if (depthBuffer[x] <= depth) {
                            uint16_t dst = line[x];
                            uint8_t dstR = RGB565_TO_RED(dst);
                            uint8_t dstG = RGB565_TO_GREEN(dst);
                            uint8_t dstB = RGB565_TO_BLUE(dst);
                            uint8_t srcR = RGB565_TO_RED(src);
                            uint8_t srcG = RGB565_TO_GREEN(src);
                            uint8_t srcB = RGB565_TO_BLUE(src);
                            dstR = dstR + srcR < 31 ? dstR + srcR : 31;
                            dstG = dstG + srcG < 63 ? dstG + srcG : 63;
                            dstB = dstB + srcB < 31 ? dstB + srcB : 31;
                            uint16_t result = RGB565RAW(dstR,dstG,dstB);
                            line[x] = result;
                            depthBuffer[x] = depth;
                        }
                    }
                }
                break;
            }
            break;
        case RenderCommandType::textured:
            rect.texture->fillLine(line, rect.x1, rect.u, rect.v + y - y1, rect.x2 - rect.x1, rect.blendMode,depthBuffer,depth);
            break;
        case RenderCommandType::text:
            fillLineText(line, y,depthBuffer);
            break;
        }
    }
}

// make sure we generate all function with uint16 / uint8 signture
template class Texture<uint16_t>;
template class RenderCommand<uint16_t>;

template class Texture<uint8_t>;
template class RenderCommand<uint8_t>;
