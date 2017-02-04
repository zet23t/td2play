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
        bool write(const void *data, int size) {
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
        bool read(void *data, int size) {
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
        void seek(int pos) {
        }
        void seekEnd(int pos) {
        }
    };
    #else
    class Persistence {
        FILE *fp;
    public:

        bool isInitialized;
        bool init(const char *baseDir) {
            isInitialized = true;

            fp = fopen(baseDir, "rb+");
            if (fp == 0) {
                fp = fopen(baseDir, "wb+");
            }
            return fp != 0;
        }
        bool write(const void *data, int size) {
            fwrite(data,size,1,fp);
            return ferror(fp) == 0;
        }
        bool read(void *data, int size) {
            long pos = ftell(fp);
            long s = fseek(fp,0, SEEK_END);
            fseek(fp,pos,SEEK_SET);
            if (pos +size > s) {
                return 0;
            }

            fread(data, size, 1, fp);
            return ferror(fp) == 0;
        }
        void close() {
            fclose(fp);
            fp = 0;
            isInitialized = false;
        }
        void seek(int pos) {
            fseek(fp,pos,SEEK_SET);
        }
        void seekEnd(int pos) {
            fseek(fp,pos,SEEK_END);
        }
    };
    #endif // __WIN32__



    Persistence persistence[MAX_PERSISTENCE_COUNT];


    Persistence* init(const char *baseDir) {
        for (int i=0;i<MAX_PERSISTENCE_COUNT;i+=1) {
            if (persistence[i].isInitialized == false) {
                persistence[i].init(baseDir);
                return &persistence[i];
            }
        }
        return 0;
    }

    bool read(Persistence *p, void *data, int size) {
        return p->read(data,size);
    }

    bool write(Persistence *p, const void *data, int size) {
        return p->write(data, size);
    }

    bool seek(Persistence *p, int pos) {
        //p->seek(pos);
    }

    bool seekEnd(Persistence *p, int pos) {
        p->seekEnd(pos);
    }

    void close(Persistence *p) {
        p->close();
    }
}


