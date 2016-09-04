#ifndef __LIB_STORAGE_H__
namespace Storage {
    void init(const char *baseDir);
    void write(const char *key, void *data, int size);
    void read(const char *key, void *data, int size);
}
#endif // __LIB_STORAGE_H__
