#include <SPI.h>
#include <Wire.h>
#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"
#include "asset_tilemap.h"
#include "image_data.h"
TinyScreen display = TinyScreen(0);
#define RENDER_COMMAND_COUNT 200
RenderBuffer<uint16_t,RENDER_COMMAND_COUNT> buffer;
const Texture<uint16_t> tiles(ImageAsset::tilemap);
TileMap::SceneRenderer<uint16_t,RENDER_COMMAND_COUNT> renderer;
TileMap::Scene<uint16_t> tilemap;

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


    tilemap = TilemapAsset::testmap();
}

void loop() {
    static unsigned int x = 0;
    x+=1;
    //buffer.drawRect(4,4,100,128)->sprite(&tiles);
    renderer.update(buffer, tilemap, x%tilemap.calcWidth()+48, 32);
    renderer.update(buffer, tilemap, x%tilemap.calcWidth()+48 - tilemap.calcWidth(), 32);
    buffer.flush(display);
    stringBuffer.reset();
}
