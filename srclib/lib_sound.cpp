#include "lib_sound.h"
#include <inttypes.h>

#define BUFFER_SAMPLE_COUNT (2048)
#define FREQUENCY (11025)


#ifdef WIN32

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

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

#define MAX_SAMPLE_PLAYBACKS 16

namespace Sound {
    struct SamplePlayback {
        int8_t *samples;
        uint16_t remaining;
        uint16_t speed;
        uint16_t remainder;
        void init(int8_t *s, uint16_t len, uint16_t speed);
    };

    void SamplePlayback::init(int8_t *s, uint16_t len, uint16_t speed) {
        samples = s;
        remaining = len;
        remainder = 0;
        this->speed = speed;
    }

    int8_t sampleBuffer[BUFFER_SAMPLE_COUNT];
    uint16_t playbackPosition;
    uint16_t bufferPosition;
    static SamplePlayback playbacks[MAX_SAMPLE_PLAYBACKS];

    void init() {
        NATIVE_INIT();
    }
    void playSample(int8_t *samples, uint16_t length, uint16_t speed) {
        for (int i=0;i<MAX_SAMPLE_PLAYBACKS;i+=1)
        {
            if (playbacks[i].remaining == 0) {
                playbacks[i].init(samples, length, speed);
                break;
            }
        }
    }
    void tick() {
        playbackPosition%=BUFFER_SAMPLE_COUNT;
        while((bufferPosition+1)%BUFFER_SAMPLE_COUNT!=playbackPosition) {
            sampleBuffer[bufferPosition] = bufferPosition % (bufferPosition %128 + 1) % 16-8;
            bufferPosition = (bufferPosition+1) % BUFFER_SAMPLE_COUNT;
        }
        NATIVE_UPDATE(sampleBuffer, &bufferPosition, &playbackPosition);
    }

}
