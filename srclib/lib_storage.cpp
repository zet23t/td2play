#ifndef __WIN32__


#include <SPI.h>
#include <SD.h>

namespace Storage {
    void init(const char *baseDir) {
    }
    void write(const char *key, void *data, int size) {
    }
    void read(const char *key, void *data, int size) {

    }

}

#endif
