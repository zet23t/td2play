#include "lib_StringBuffer.h"
#include "lib_storage.h"
#ifndef __WIN32__

#include <SPI.h>
#include <SD.h>

#else

#include <stdio.h>

#endif

namespace Storage {
    #ifndef __WIN32__
    struct Persistence {
        bool isInitialized;
        const char *gBaseDir;

        bool init(const char *baseDir) {
            gBaseDir = baseDir;
            isInitialized = true;
            if (!SD.begin(10)) {
                isInitialized = false;
                return false;
            }
            return true;
        }
        bool write(const void *data, int pos, int size) {
            /*stringBuffer.start().put(gBaseDir).put('/').put(file);
            File myFile = SD.open(stringBuffer.getAndForget(), FILE_WRITE);
            if (myFile) {
                myFile.write((uint8_t*)data, size);
                myFile.flush();
                myFile.close();
                return true;
            }*/
            return false;
        }
        bool read(void *data, int pos, int size) {
            /*stringBuffer.start().put(gBaseDir).put('/').put(file);
            File myFile = SD.open(stringBuffer.getAndForget(), FILE_READ);
            if (myFile) {
                myFile.read(data, size);
                myFile.close();
                return true;
            }*/
            return false;
        }
        void close() {
            isInitialized = false;
        }
    };
    #else

    bool Persistence::init(const char *baseDir) {
        fp = fopen(baseDir, "rb+");
        if (fp == 0) {
            fp = fopen(baseDir, "wb+");
        }
        return fp != 0;
    }
    bool Persistence::write(const void *data, int pos, int size) {
        fwrite(data,size,1,fp);
        return ferror(fp) == 0;
    }
    bool Persistence::read(void *data, int pos, int size) {

        fseek(fp,pos,SEEK_SET);

        fread(data, size, 1, fp);
        return ferror(fp) == 0;
    }
    void Persistence::close() {
        fclose(fp);
        fp = 0;
    }
    #endif // __WIN32__


    void init(Persistence& p, const char *baseDir) {
        p.init(baseDir);

    }

    bool read(Persistence *p, void *data, int pos, int size) {
        return p->read(data, pos,size);
    }

    bool write(Persistence *p, const void *data, int pos, int size) {
        return p->write(data, pos, size);
    }
}


