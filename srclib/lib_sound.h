#pragma once

#include <inttypes.h>

extern "C" {
    #define TD2PLAY 1
    #include "lib_hxcmod.h"
}

namespace Sound {
    void playSample(int8_t *samples, uint16_t length, uint16_t speed);
    void init();
    void tick();
}
