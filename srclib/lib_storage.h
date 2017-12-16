#ifndef __LIB_STORAGE_H__

#define MAX_PERSISTENCE_COUNT 4

#ifndef __WIN32__

#include <SPI.h>
#include <SD.h>

#else

#include <stdio.h>

#endif

namespace Storage {
    struct Persistence {
        #ifdef WIN32
        FILE *fp;
        #else
        File file;
        bool isInitialized;
        #endif // WIN32

        bool init(const char *baseDir);
        bool write(const void *data, int pos, int size);
        bool read(void *data, int pos, int size);
        void close();
    };
}
#endif // __LIB_STORAGE_H__
