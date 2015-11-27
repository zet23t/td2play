#include "lib_RenderBuffer.h"

void Texture::fillLineRgb565sram (uint16_t *lineBuffer, uint8_t lineX, uint16_t u, uint16_t v, uint8_t width, uint8_t blendMode) const  {
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

Texture::Texture (uint8_t *data, uint8_t type, uint16_t width, uint16_t height, uint16_t transparentColorMask) {
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

void Texture::fillLine(uint16_t *lineBuffer, uint8_t lineX, uint8_t u, uint8_t v, uint8_t width, uint8_t blendMode) const {
    switch (type) {
    case TextureType::rgb565sram: fillLineRgb565sram(lineBuffer,lineX,u,v,width, blendMode); break;
    }
}
