#ifndef __LIB_STORAGE_H__

#define MAX_PERSISTENCE_COUNT 4

namespace Storage {
    class Persistence;

    Persistence* init(const char *baseDir);
    bool read(Persistence *p, void *data, int size);
    bool write(Persistence *p, const void *data, int size);
    bool seek(Persistence *p, int pos);
    bool seekEnd(Persistence *p, int pos);
    void close(Persistence *p);
}
#endif // __LIB_STORAGE_H__
