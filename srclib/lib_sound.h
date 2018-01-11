#pragma once

#include <inttypes.h>

extern "C" {
    #define TD2PLAY 1
    #include "lib_hxcmod.h"
}

namespace Sound {
    void playSample(uint16_t id, const int8_t *samples, uint16_t length, uint16_t speed, uint16_t volume, uint16_t loops);
    void stopSample(int16_t id);
    void updateVolume(int16_t id, uint16_t volume);
    void updateSpeed(int16_t id, uint16_t speed);
    void init();
    void tick();
}
