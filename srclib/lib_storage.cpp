#include "lib_StringBuffer.h"
#include "lib_storage.h"


namespace Storage {
    #ifndef WIN32
    bool Persistence::init(const char *baseDir) {
        isInitialized = true;
        if (!SD.begin(10)) {
            isInitialized = false;
            return false;
        }
        file = SD.open(baseDir);
        isInitialized = (bool)file;
        return isInitialized;
    }
    bool Persistence::write(const void *data, int pos, int size) {
        if (file) {
            file.seek(pos);
            file.write((uint8_t*)data, size);
            file.flush();
        }
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
    bool Persistence::read(void *data, int pos, int size) {
        if (file) {
            file.seek(pos);
            file.read((uint8_t*)data, size);
            return true;
        }
        /*stringBuffer.start().put(gBaseDir).put('/').put(file);
        File myFile = SD.open(stringBuffer.getAndForget(), FILE_READ);
        if (myFile) {
            myFile.read(data, size);
            myFile.close();
            return true;
        }*/
        return false;
    }
    void Persistence::close() {
        if (isInitialized) {
            file.close();
        }
        isInitialized = false;
    }

    #else

    bool Persistence::init(const char *baseDir) {
        fp = fopen(baseDir, "rb+");
        if (fp == 0) {
            // sd arduino lib expects preceding slash - that's not what we need on windows
            fp = fopen(&baseDir[1], "wb+");
            if (fp == 0) {
                printf("WARNING: Could not open file %s\n",baseDir);
            }
        }
        return fp != 0;
    }
    bool Persistence::write(const void *data, int pos, int size) {
        if (fp == 0) return false;
        fwrite(data,size,1,fp);
        return ferror(fp) == 0;
    }
    bool Persistence::read(void *data, int pos, int size) {
        if (fp == 0) return false;
        fseek(fp,pos,SEEK_SET);

        fread(data, size, 1, fp);
        return ferror(fp) == 0;
    }
    void Persistence::close() {
        if (fp == 0) fclose(fp);
        fp = 0;
    }
    #endif // __WIN32__

}


