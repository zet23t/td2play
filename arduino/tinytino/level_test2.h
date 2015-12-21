#ifndef __LEVEL_TEST2_H__
#define __LEVEL_TEST2_H__

namespace Level_test2 {
extern TileMap::ProgmemData getForeground();
extern TileMap::ProgmemData getBackground();
extern TileMap::TileSetBgFg<uint16_t> getTileset();
extern TileMap::SceneBgFg<uint16_t> getScene();

}

#endif // __LEVEL_TEST2_H__
