#ifndef __GAME_TINYRUN_H__
#define __GAME_TINYRUN_H__

#include "lib_RenderBuffer.h"
#include "lib_StringBuffer.h"

#define RENDER_COMMAND_COUNT 340

extern RenderBuffer<uint16_t,RENDER_COMMAND_COUNT> buffer;

namespace Game {
    void setup(void);
    void loop();
}

#endif // __GAME_TINYRUN_H__
