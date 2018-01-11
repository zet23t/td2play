#include "lib_sound.h"
#include <inttypes.h>

#define BUFFER_SAMPLE_COUNT (2048)
#define FREQUENCY (11025)
#include <memory.h>


#ifdef WIN32

#include <stdlib.h>
#include <stdio.h>

#include <al.h>
#include <alc.h>

///// sound stuff - source: https://stackoverflow.com/questions/5469030/c-play-back-a-tone-generated-from-a-sinusoidal-wave
#define CASE_RETURN(err) case (err): return "##err"
const char* al_err_str(ALenum err) {
    switch(err) {
        CASE_RETURN(AL_NO_ERROR);
        CASE_RETURN(AL_INVALID_NAME);
        CASE_RETURN(AL_INVALID_ENUM);
        CASE_RETURN(AL_INVALID_VALUE);
        CASE_RETURN(AL_INVALID_OPERATION);
        CASE_RETURN(AL_OUT_OF_MEMORY);
    }
    return "unknown";
}
#undef CASE_RETURN
#define __al_check_error(file,line) \
    do { \
        ALenum err = alGetError(); \
        for(; err!=AL_NO_ERROR; err=alGetError()) { \
            printf("AL Error %s at %s:%d\n",al_err_str(err),file,":",line); \
        } \
    }while(0)

#define al_check_error() \
    __al_check_error(__FILE__, __LINE__)

ALuint src = 0;
static ALuint buf[2];
static uint8_t sampleBuffer[BUFFER_SAMPLE_COUNT/2];

void init_al() {
    ALCdevice *dev = NULL;
    ALCcontext *ctx = NULL;

    const char *defname = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    printf("Default device: %s\n", defname);

    dev = alcOpenDevice(defname);
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);


    alGenBuffers(2, buf);
    al_check_error();
    alGenSources(1, &src);
    memset(sampleBuffer,0,sizeof(sampleBuffer));
    for (int i = 0; i < 2; i++)
    {
       alBufferData(buf[i], AL_FORMAT_MONO8, sampleBuffer, sizeof(sampleBuffer), FREQUENCY);
       alSourceQueueBuffers(src, 1, &buf[i]);
    }
    alSourcePlay(src);
}


void update_al(int8_t* buffer, uint16_t* bufferPos, uint16_t* playbackPos) {
    int pcnt;
    alGetSourcei(src, AL_BUFFERS_PROCESSED, &pcnt);
    for (int i=0;i<pcnt;i+=1) {
        ALuint buf;
        alSourceUnqueueBuffers(src, 1, &buf);
        int remaining = sizeof(sampleBuffer);
        int p = 0;
        memset(sampleBuffer,0,sizeof(sampleBuffer));
        //printf("%d %d\n",*playbackPos, *bufferPos);
        while (*playbackPos != *bufferPos && p < sizeof(sampleBuffer)) {
            int8_t s = buffer[*playbackPos];
            sampleBuffer[p++] = (uint8_t)(s + 127);
            buffer[*playbackPos] = 0;
            //printf(" %d",s);
            *playbackPos = (*playbackPos + 1) % BUFFER_SAMPLE_COUNT;
        }
        //printf("\n");

        alBufferData(buf, AL_FORMAT_MONO8, sampleBuffer, sizeof(sampleBuffer), FREQUENCY);
        alSourceQueueBuffers(src, 1, &buf);

    }
}


#define NATIVE_INIT init_al
#define NATIVE_UPDATE update_al

#endif // WIN32

#define MAX_SAMPLE_PLAYBACKS 8

namespace Sound {
    struct SamplePlayback {
        const int8_t *samples;
        uint32_t pos;
        uint16_t length;
        uint16_t speed;
        uint16_t volume;
        uint16_t loopCount;
        uint16_t id;
        void stop();
        void fillBuffer(int8_t *s, uint16_t n);
        void init(const int8_t *s, uint16_t len, uint16_t speed, uint16_t volume, uint16_t loops, uint16_t id);
    };
    void SamplePlayback::stop() {
        samples = 0;
    }
    void SamplePlayback::init(const int8_t *s, uint16_t len, uint16_t speed, uint16_t v, uint16_t loops, uint16_t id) {
        samples = s;
        length = len;
        pos = 0;
        this->speed = speed;
        this->id = id;
        loopCount = loops;
        volume = v;
    }
    void SamplePlayback::fillBuffer(int8_t *buf, uint16_t n) {
        if (!samples) return;
        uint32_t len = length << 8;
        for (int i=0;i<n;i+=1) {
            uint16_t p = pos >> 8;
            int32_t a = samples[p];
            int32_t b = samples[p + 1 < length ? p + 1 : 0];
            uint8_t mix = pos & 0xff;
            int32_t res = (a * (0xff-mix) + b * mix) >> 8;
            //printf("%d ",res);
            int32_t s = res + buf[n];
            //printf("%d %d\n",samples[pos>>8],pos>>8);
            //pos += speed;
            s = (s * volume) >> 8;
            if (s < -126) s = -126;
            else if (s > 127) s = 127;
            buf[i] = (int8_t) s;
            pos += speed;
            while (pos >= (len)) {
                if (loopCount <= 1) {
                    stop();
                    return;
                }
                if (loopCount < 0xffff) loopCount -= 1;
                pos -= len;
            }
        }
    }

    int8_t sampleBuffer[BUFFER_SAMPLE_COUNT];
    uint16_t playbackPosition; // position that is played / queued on sound out
    uint16_t bufferPosition; // position we have filled the buffer to
    static SamplePlayback playbacks[MAX_SAMPLE_PLAYBACKS];

    void fillBuffer(uint16_t from, uint16_t n) {
        //printf("%d %d %d %d\n",from,n,playbackPosition, bufferPosition);
        memset(&sampleBuffer[from],0,n);
        for (int i=0;i<MAX_SAMPLE_PLAYBACKS;i+=1) {
            playbacks[i].fillBuffer(&sampleBuffer[from], n);
        }
        //for (int i=from;i<from+n;i+=1)
        //    sampleBuffer[i] = i % (i%128 + 1) % 16-8;
    }
    void fillBuffer() {
        playbackPosition%=BUFFER_SAMPLE_COUNT;
        uint16_t pos = playbackPosition;
        if (pos <= bufferPosition) {
            if (bufferPosition < BUFFER_SAMPLE_COUNT) {
                fillBuffer(bufferPosition,BUFFER_SAMPLE_COUNT - bufferPosition);
            }
            if (pos > 1) {
                fillBuffer(0, pos - 1);
                bufferPosition = pos - 1;
            } else {
                bufferPosition = BUFFER_SAMPLE_COUNT;
            }
        } else {
            if (pos - bufferPosition > 1) {
                fillBuffer(bufferPosition, pos-bufferPosition-1);
                bufferPosition = pos-1;
            }
        }

    }
    void init() {
        NATIVE_INIT();
        fillBuffer();
        //static const int8_t samples[] = {100,-100};//,80,-80,50,-50,10,-10};
        //playSample(1,samples, sizeof(samples), 0x20,0x200,0xff);
    }
    void playSample(uint16_t id, const int8_t *samples, uint16_t length, uint16_t speed, uint16_t volume, uint16_t loops) {
        for (int i=0;i<MAX_SAMPLE_PLAYBACKS;i+=1)
        {
            if (playbacks[i].length == 0) {
                playbacks[i].init(samples, length, speed, volume, loops,id);
                break;
            }
        }
    }
    void stopSample(uint16_t id) {
        for (int i=0;i<MAX_SAMPLE_PLAYBACKS;i+=1)
        {
            if (playbacks[i].id == id) {
                playbacks[i].stop();
            }
        }
    }
    void updateVolume(int16_t id, uint16_t volume) {
        for (int i=0;i<MAX_SAMPLE_PLAYBACKS;i+=1)
        {
            if (playbacks[i].id == id) {
                playbacks[i].volume = volume;
            }
        }
    }
    void updateSpeed(int16_t id, uint16_t speed) {
        for (int i=0;i<MAX_SAMPLE_PLAYBACKS;i+=1)
        {
            if (playbacks[i].id == id) {
                playbacks[i].speed = speed;
            }
        }
    }
    void tick() {
        fillBuffer();
        NATIVE_UPDATE(sampleBuffer, &bufferPosition, &playbackPosition);
    }

}
