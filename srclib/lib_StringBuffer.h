#ifndef __STRINGBUFFER_H__
#define __STRINGBUFFER_H__
#define STRINGBUFFER_SIZE 256

#include <string.h>
#include <inttypes.h>

#ifndef WIN32
#include <avr/pgmspace.h>
#endif // WIN32

#ifdef WIN32
#define PSTR
#define strlen_P strlen
#define strcpy_P strcpy
#endif // WIN32

class StringBuffer {
private:
    char buffer[STRINGBUFFER_SIZE];
    uint8_t bufferPos;
    uint8_t currentPos;
public:
    void reset() {
        currentPos = 0;
        bufferPos = 0;
        memset(buffer,0,STRINGBUFFER_SIZE);
    }
    StringBuffer& start() {
        currentPos = bufferPos;
        return *this;
    }
    StringBuffer& put(const char* str) {
        while (*str && bufferPos < STRINGBUFFER_SIZE) {
            buffer[bufferPos++] = *str;
            str+=1;
        }
        return *this;
    }
    StringBuffer& put(char letter) {
        if (bufferPos < STRINGBUFFER_SIZE) buffer[bufferPos++] = letter;
        return *this;
    }
    StringBuffer& putDec(int32_t num) {
        const char digit[] = "0123456789";
        char decs[32];
        uint8_t pos = 0;
        uint8_t neg = num < 0;
        if (neg) num = -num;
        do {
            uint8_t dec = num%10;
            num/=10;
            decs[pos++] = digit[dec];
        } while (num != 0);
        if (neg) decs[pos++]= '-';
        while (pos-- > 0 && bufferPos < STRINGBUFFER_SIZE) buffer[bufferPos++] = decs[pos];
        return *this;
    }
    StringBuffer& load(const char *src) {
        int len = strlen_P(src);
		if (STRINGBUFFER_SIZE - bufferPos > len) {
			char *str = &buffer[bufferPos];
			bufferPos += len;
			strcpy_P(str, src);
		} else {
		}
		return *this;
    }
    char* get() {
        buffer[bufferPos++] = 0;
        return &buffer[currentPos];
    }
    char* getAndForget() {
        buffer[bufferPos++] = 0;
        bufferPos = currentPos;
        return &buffer[currentPos];
    }
};

extern StringBuffer stringBuffer;

#endif
