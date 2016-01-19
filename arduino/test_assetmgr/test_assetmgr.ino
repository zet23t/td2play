#include <SPI.h>
#include <Wire.h>
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"

#include "image_data.h"
TinyScreen display = TinyScreen(0);

RenderBuffer<uint16_t,5> buffer;
const Texture<uint16_t> tiles(ImageAsset::tilemap);

void setup() {
    Wire.begin();
    #if defined(ARDUINO_ARCH_SAMD)
    display.begin(TinyScreenPlus);
    #else
    display.begin();
    #endif

    display.setFlip(0);
    display.setBrightness(8);
    display.setBitDepth(buffer.is16bit());
}

void loop() {
    buffer.drawRect(4,4,100,128)->sprite(&tiles);
    buffer.flush(display);
    stringBuffer.reset();
}
