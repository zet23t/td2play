#ifndef __LIB_STORAGE_H__

#define MAX_PERSISTENCE_COUNT 4

#ifdef WIN32
#include <stdio.h>
#endif // WIN32

namespace Storage {
    struct Persistence {
        #ifdef WIN32
        FILE *fp;
        #endif // WIN32

        bool init(const char *baseDir);
        bool write(const void *data, int pos, int size);
        bool read(void *data, int pos, int size);
        void close();
    };

    void init(Persistence &p, const char *baseDir);
    bool read(Persistence *p, void *data,int pos, int size);
    bool write(Persistence *p, const void *data, int pos, int size);
    void close(Persistence *p);
}
#endif // __LIB_STORAGE_H__
