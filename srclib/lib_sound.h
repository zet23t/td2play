#pragma once

#include <inttypes.h>
#define FREQUENCY (11025)

extern "C" {
    #define TD2PLAY 1
    #include "lib_hxcmod.h"
}

namespace Sound {
    struct SamplePlayback;

    typedef void (*OnSoundStopCallback)(SamplePlayback* s);

    struct SamplePlayback {
        const int8_t *samples;
        uint32_t pos;
        uint16_t length;
        uint16_t speed;
        uint16_t volume;
        uint16_t loopCount;
        uint32_t changeTimer;
        uint32_t changeInterval;
        int16_t changeVolume;
        int16_t changeSpeed;
        int8_t interpolate;
        void* data;
        OnSoundStopCallback callback;
        uint16_t id;
        void stop();
        void fillBuffer(int8_t *s, uint16_t n);
        SamplePlayback* enableInterpolate();
        SamplePlayback* setChange(uint32_t interval, int16_t vol, int16_t speed);
        void init(const int8_t *s, uint16_t len, uint16_t speed, uint16_t volume, uint16_t loops, uint16_t id);
        SamplePlayback* setOnStopCallback(OnSoundStopCallback cb, void* dat);
    };

    SamplePlayback* playSample(uint16_t id, const int8_t *samples, uint16_t length, uint16_t speed, uint16_t volume, uint16_t loops);
    void stopSample(uint16_t id);
    void updateVolume(int16_t id, uint16_t volume);
    void updateSpeed(int16_t id, uint16_t speed);

    void init();
    void tick();
}
