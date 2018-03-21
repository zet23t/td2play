#include "lib_flashstore.h"

#define FLASHSTORE_START 0xA000
#define FLASHSTORE_END 0xF000
#define FLASHSTORE_MAGIC 0xfa3d
#include "string.h"

#if WIN32
#include "stdio.h"
#else

#endif

namespace FlashStore {

    uint32_t calcChecksum(const uint16_t sz, const uint8_t* data) {
        uint32_t sum = 0;
        for (int i=0;i<sz;i+=1) {
            sum ^= (sum << 3) ^ (sum >> 1) ^ ((data[i] + i + 0x3a7719) << (i&127));
        }
        return sum;
    }

    struct Chunk {
        uint32_t gameId;
        uint32_t checkSum;
        uint16_t magic;
        uint16_t dataSize;
        uint8_t storageVersion;
        char gameName[19];

        void init(uint32_t id, const char* name, uint16_t dsize, void *data) {
            gameId = id;
            magic = FLASHSTORE_MAGIC;
            dataSize = dsize;
            for(int i=0;i<sizeof(gameName); i+=1) {
                gameName[i] = name[i];
                if (!name[i]) {
                    while (i<sizeof(gameName)) gameName[i++] = 0;
                }
            }
            storageVersion = 1;
            checkSum = 0; // this is important for the checksum calculation!
            checkSum = (calcChecksum(sizeof(Chunk), (const uint8_t*)this)&0xff)
                | (calcChecksum(dsize,(uint8_t*)data)&0xffffff00);
        }
        void println() const {
            printf("id: %d name: %s\n", gameId, gameName);
            printf("magic: %p checksum: %p\n", magic, checkSum);
        }

        bool isHeaderValid() const {
            Chunk chk = *this;
            chk.checkSum = 0;
            uint32_t sum = calcChecksum(sizeof(Chunk),(const uint8_t*)&chk);
            //printf("%p %p\n",sum,checkSum);

            //println();
            return (checkSum & 0xff) == (sum & 0xff);
        }

        bool isDataValid(uint16_t siz, const void* data) const {
            uint32_t sum = calcChecksum(siz,(const uint8_t*)data);
            return (checkSum & 0xffffff00) == (sum & 0xffffff00);
        }
    };

    bool findPosOf(uint8_t *block, uint32_t gameId, uint16_t dataSize, Chunk** outResult) {
        int p = 0;
        while (p < FLASHSTORE_END - FLASHSTORE_START - sizeof(Chunk) - dataSize) {
            Chunk* chunk = (Chunk*)&block[p];
            if (chunk->magic != FLASHSTORE_MAGIC) {
                *outResult =chunk;
                return false;
            }
            if (chunk->gameId == gameId && chunk->dataSize == dataSize) {
                *outResult = chunk;
                return true;
            }
            p+=sizeof(Chunk) + chunk->dataSize;
        }
        *outResult = 0;
        return false;
    }

    #if WIN32

    bool storeImpl(uint32_t gameId, const char* gameName, uint16_t dataSize, void* data) {
        FILE *f = fopen("flash.bin","rb");
        uint8_t allData[0x10000];
        if (f) {
            memset(allData,0,sizeof(allData));
            fread(allData, 1, sizeof(allData), f);
            fclose(f);
        }
        Chunk *chunk;
        bool valid = findPosOf(&allData[FLASHSTORE_START], gameId, dataSize, &chunk);
        if (!valid && chunk == 0) return false;
        chunk->init(gameId, gameName, dataSize, data);
        uint8_t *dataStore = (uint8_t*)&chunk[1];
        uint8_t *intData = (uint8_t*)data;
        for (int i=0;i<dataSize;i+=1) {
            dataStore[i] = intData[i];
        }
        f = fopen("flash.bin","wb");
        if (!f) return 0;
        fwrite(allData,1,sizeof(allData),f);
        fclose(f);
    }
    bool restoreImpl(uint32_t gameId, uint16_t dataSize, void *result) {
        FILE *f = fopen("flash.bin","rb");
        if (!f) {
            printf("Flashstore::restore: file not present\n");
            return 0;
        }
        uint8_t allData[0x10000];
        memset(allData,0,sizeof(allData));
        fread(allData, 1, sizeof(allData), f);
        fclose(f);
        Chunk *chunk;
        bool valid = findPosOf(&allData[FLASHSTORE_START], gameId, dataSize, &chunk);
        if (!valid || chunk == 0 || !chunk->isHeaderValid() || !chunk->isDataValid(dataSize, (void*)&chunk[1])) {
            if (!chunk)
                printf("Flashstore::restore: no such game id found\n");
            else {
                printf("Flashstore::restore: validity check failed: %d, %d\n",
                       chunk->isHeaderValid(), chunk->isDataValid(dataSize, (void*)&chunk[1]));

            }
            return false;
        }
        uint8_t* src = (uint8_t*)&chunk[1];
        uint8_t* dst = (uint8_t*)result;

        for (int i=0;i<dataSize;i+=1) {
            dst[i] = src[i];
        }
        return true;
    }

    #else

    void* findPos(uint32_t gameId, uint16_t dataSize) {
        void *p = (void*)FLASHSTORE_START;
        while (p < FLASHSTORE_END) {

        }
    }

    #endif // WIN32

    bool store(uint32_t gameId, const char* gameName, uint16_t dataSize, void* data) {
        return storeImpl(gameId,gameName, dataSize, data);
    }
    bool restore(uint32_t gameId, uint16_t dataSize, void *result) {
        return restoreImpl(gameId,dataSize,result);
    }
}
