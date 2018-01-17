#include "lib_sound.h"
#include <inttypes.h>

#define BUFFER_SAMPLE_COUNT (1024)

#include <string.h>


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
        int p = 0;
        memset(sampleBuffer,0,sizeof(sampleBuffer));
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
    ALenum state;
    alGetSourcei(src, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING)
        alSourcePlay(src);
}


#define NATIVE_INIT init_al
#define NATIVE_UPDATE update_al

#else // arduino init

#include <Wire.h>
#include <SPI.h>

#ifdef __cplusplus
extern "C" {
#endif
int8_t *audioBuffer;
uint16_t *audioBufferPos;

void TC5_Handler (void)
{

  int v = audioBuffer ? audioBuffer[*audioBufferPos] + 127 : 127;
  if (audioBuffer) {
      *audioBufferPos+=1;
      if( *audioBufferPos > BUFFER_SAMPLE_COUNT )
        *audioBufferPos = 0;
  }

  while( DAC->STATUS.bit.SYNCBUSY == 1 );
  DAC->DATA.reg = v<<2;
  while( DAC->STATUS.bit.SYNCBUSY == 1 );

  // Clear the interrupt
  TC5->COUNT16.INTFLAG.bit.MC0 = 1;
}
#ifdef __cplusplus
}
#endif

bool tcIsSyncing()
{
  return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

void tcStart()
{
  // Enable TC
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
  while( tcIsSyncing());
}

void tcReset()
{
  // Reset TCx
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while( tcIsSyncing());
  while( TC5->COUNT16.CTRLA.bit.SWRST );
}

void tcStop()
{
  // Disable TC5
  TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while( tcIsSyncing());
}

void tcConfigure( uint32_t sampleRate )
{
  // Enable GCLK for TCC2 and TC5 (timer counter input clock)
  GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5)) ;
  while (GCLK->STATUS.bit.SYNCBUSY);

  tcReset();

  // Set Timer counter Mode to 16 bits
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;

  // Set TC5 mode as match frequency
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;

  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1;

  TC5->COUNT16.CC[0].reg = (uint16_t) (SystemCoreClock / sampleRate - 1);
  while (tcIsSyncing());

  // Configure interrupt request
  NVIC_DisableIRQ(TC5_IRQn);
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 0);
  NVIC_EnableIRQ(TC5_IRQn);

  // Enable the TC5 interrupt request
  TC5->COUNT16.INTENSET.bit.MC0 = 1;
  while( tcIsSyncing());
}

void update_tc(int8_t* buffer, uint16_t* bufferPos, uint16_t* playbackPos) {
    audioBuffer = buffer;
    audioBufferPos = playbackPos;
}

void init_tc() {
    tcConfigure( FREQUENCY );
    tcStart();
}

#define NATIVE_INIT init_tc
#define NATIVE_UPDATE update_tc


#endif // WIN32

#define MAX_SAMPLE_PLAYBACKS 8

namespace Sound {
    void SamplePlayback::stop() {
        if (callback) {
            callback(this);
        }
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
        interpolate = 0;
        changeInterval = 0;
        callback = 0;
        data = 0;
    }
    SamplePlayback* SamplePlayback::setOnStopCallback(OnSoundStopCallback cb, void* dat) {
        callback = cb;
        data = dat;
        return this;
    }
    void SamplePlayback::fillBuffer(int8_t *buf, uint16_t n) {
        if (!samples) return;
        uint32_t len = length << 8;
        for (int i=0;i<n;i+=1) {
            uint16_t p = pos >> 8;
            int32_t a = samples[p];
            int32_t res;
            if (interpolate) {
                int32_t b = samples[p + 1 < length ? p + 1 : 0];
                uint8_t mix = pos & 0xff;
                res = (a * (0xff-mix) + b * mix) >> 8;

            } else {
                res = a;
            }
            //printf("%d ",res);
            int32_t s = res + buf[n];
            //printf("%d %d\n",samples[pos>>8],pos>>8);
            //pos += speed;
            s = (s * volume) >> 8;
            if (s < -126) s = -126;
            else if (s > 127) s = 127;
            buf[i] = (int8_t) s;
            pos += speed;
            if (changeInterval) {
                changeTimer += speed;
                while (speed > 0 && changeTimer >= changeInterval) {
                    changeTimer -= changeInterval;
                    if (speed >= -changeSpeed) speed += changeSpeed;
                    else speed = 0;
                    if (volume >= -changeVolume) volume += changeVolume;
                    else volume = 0;
                }
                if (speed == 0 || volume == 0) {
                    stop();
                    return;
                }
            }
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

    SamplePlayback* SamplePlayback::enableInterpolate(){
        interpolate = 1;
        return this;
    }
    SamplePlayback* SamplePlayback::setChange(uint32_t interval, int16_t vol, int16_t speed){
        changeSpeed = speed;
        changeVolume = vol;
        changeInterval = interval;
        return this;
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
    SamplePlayback* playSample(uint16_t id, const int8_t *samples, uint16_t length, uint16_t speed, uint16_t volume, uint16_t loops) {
        for (int i=0;i<MAX_SAMPLE_PLAYBACKS;i+=1)
        {
            if (playbacks[i].samples == 0) {
                playbacks[i].init(samples, length, speed, volume, loops,id);
                return &playbacks[i];
            }
        }
        static SamplePlayback def;
        return &def;
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
