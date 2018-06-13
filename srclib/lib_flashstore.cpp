#include "lib_flashstore.h"

#define FLASHSTORE_START 0
#define FLASHSTORE_END (1024*8)
#define FLASHSTORE_MAGIC 0xfa3d
#include "string.h"

#include "stdio.h"

#ifndef WIN32
#include <Wire.h>
#include <SPI.h>
#include <TinyScreen.h>
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
        if (!f) return false;
        fwrite(allData,1,sizeof(allData),f);
        fclose(f);
        return true;
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



TinyScreen display = TinyScreen(TinyScreenPlus);
void printCentered(const char * printString, int y) {
    static bool init = false;
    if (!init) {
      display.begin();
      display.setFont(thinPixel7_10ptFontInfo);
    }
  display.setCursor(48 - ((int)display.getPrintWidth((char*)printString) / 2), y);
  display.print((char*)printString);
}

const uint32_t pageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };

#define EEPROM_START ((64-8) * 1024)

struct ReadBufferObject {
  uint8_t *buffer;
  uint32_t pos;
  int32_t size;
};

uint8_t *buffer;

void flashProgress(const uint32_t, const uint32_t, void*) {
}

int readBuffer(uint32_t PAGE_SIZE, void *udata) {
    printCentered("  Reading buffer  ",40);
  ReadBufferObject *rbo = (ReadBufferObject*)udata;
  memcpy(buffer,&rbo->buffer[rbo->pos],PAGE_SIZE);
  rbo->pos += PAGE_SIZE;
  return 0;
}

static void nvmErase(uint32_t erase_dst_addr, uint32_t MAX_FLASH, uint32_t PAGE_SIZE, void (*progress)(const uint32_t, const uint32_t, void*), void* progressData)
{
  uint32_t base = erase_dst_addr;
  while (erase_dst_addr < MAX_FLASH) {
    // Execute "ER" Erase Row
    NVMCTRL->ADDR.reg = erase_dst_addr / 2;
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;

    if (progress) progress(erase_dst_addr - base, MAX_FLASH - base, progressData);

    while (NVMCTRL->INTFLAG.bit.READY == 0) {
      // intentionally nothing
    }
    erase_dst_addr += PAGE_SIZE * 4; // Skip a ROW
  }
}

static void nvmWrite(uint32_t address, uint32_t fileSize, uint32_t PAGE_SIZE, void (*writeProgress)(const uint32_t, const uint32_t, void*), void* progressData, int (*reader)(uint32_t, void*), void* udata)
{
  // Write to flash
  uint32_t *dst_addr = (uint32_t *)(address);
  uint32_t size = fileSize;

  // Set automatic page write
  NVMCTRL->CTRLB.bit.MANW = 0;
  // Set first address
  NVMCTRL->ADDR.reg = address / 2;
  // Do writes in pages
  while (size) {

    printCentered("  writing flash  ", 40);
    reader(PAGE_SIZE, udata);

    if (size < 64) {
      for (int j = size; j < PAGE_SIZE; j++)buffer[j] = 0xFF;
    }
    // Execute "PBC" Page Buffer Clear
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
    printCentered("  wait ready", 40);
    while (NVMCTRL->INTFLAG.bit.READY == 0);

    // Fill page buffer
    printCentered("  fill buffer  ", 40);
    uint32_t i;
    for (i = 0; i < (PAGE_SIZE / 4) && i < size; i++) {
      //uint32_t data = (buffer[i + 3] << 0) | (buffer[i + 2] << 1) | (buffer[i + 1] << 2) | (buffer[i] << 3);
      dst_addr[i] = ((uint32_t *)buffer)[i];
    }

    // Execute "WP" Write Page
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;

    printCentered("  waiting  ", 40);
    writeProgress((uint32_t)dst_addr-address,fileSize,progressData);

    while (NVMCTRL->INTFLAG.bit.READY == 0);

    // Advance to next page
    dst_addr += i;
    if (size > PAGE_SIZE)
      size     -= PAGE_SIZE;
    else
      size = 0;
    printCentered("  next  ", 40);
  }
}

static bool writeEEPROM(uint8_t *buffer, uint32_t bufferSize, uint32_t offset)
{
  if (bufferSize + offset> 8*1024) return false;

  uint32_t address = EEPROM_START + offset;

  uint32_t PAGE_SIZE, PAGES, MAX_FLASH;
  PAGE_SIZE = pageSizes[NVMCTRL->PARAM.bit.PSZ];
  PAGES = NVMCTRL->PARAM.bit.NVMP;
  MAX_FLASH = PAGE_SIZE * PAGES;

  int lastDisplayedValue = -1;
  uint8_t buf[PAGE_SIZE];
  buffer = buf;


  uint32_t pageAlignedAddress = address & (~(PAGE_SIZE-1));
  const uint8_t* pagePointer = (const uint8_t*)pageAlignedAddress;
  uint32_t remainder = address - pageAlignedAddress;
  if (remainder != 0) {
    uint8_t page[PAGE_SIZE];
    memcpy(page,(const void*)pagePointer,PAGE_SIZE);
    memcpy(page+remainder,(const void*)buffer, PAGE_SIZE-remainder);

    ReadBufferObject rbo = {buffer + PAGE_SIZE-remainder,0,bufferSize - PAGE_SIZE + remainder};
    ReadBufferObject rbo2 = {page,0,PAGE_SIZE};
    nvmErase(pageAlignedAddress, address + bufferSize, PAGE_SIZE, flashProgress, 0);
    nvmWrite(pageAlignedAddress, PAGE_SIZE, PAGE_SIZE, flashProgress, (void*) 1, readBuffer, &rbo2);
    if (rbo.size > 0) {
      nvmWrite(pageAlignedAddress, rbo.size, PAGE_SIZE, flashProgress, (void*) 1, readBuffer, &rbo);
    }
  } else {
    ReadBufferObject rbo = {buffer,0,bufferSize};
    nvmErase(pageAlignedAddress, address + bufferSize, PAGE_SIZE, flashProgress, 0);
    nvmWrite(pageAlignedAddress, bufferSize, PAGE_SIZE, flashProgress, (void*) 1, readBuffer, &rbo);
  }

}

static void readEEPROM(uint8_t *buffer, uint32_t size, int32_t offset) {
  uint32_t address = EEPROM_START + offset;
  uint32_t *dst_addr = (uint32_t *)(address);
  memcpy(buffer,(const void*)address,size);
}




    bool storeImpl(uint32_t gameId, const char* gameName, uint16_t dataSize, void* data) {
        printCentered("  Begin storing   ", 48);
        Chunk *chunk;
        uint8_t *allData = 0;
        bool valid = findPosOf(&allData[EEPROM_START], gameId, dataSize, &chunk);
        if (!valid && chunk == 0) return false;

        uint8_t block[sizeof(Chunk) + dataSize];

        Chunk *chk = (Chunk*)block;
        chk->init(gameId, gameName, dataSize, data);
        uint8_t *blockData = &block[sizeof(Chunk)];
        memcpy(blockData, data, dataSize);
        writeEEPROM(block, sizeof(block), (uint32_t)chunk - EEPROM_START);
    }

    bool restoreImpl(uint32_t gameId, uint16_t dataSize, void *result) {
        uint8_t *allData = 0;
        Chunk *chunk;
        bool valid = findPosOf(&allData[EEPROM_START], gameId, dataSize, &chunk);
        if (!valid && result == 0) return false;
        result = (void*)&chunk[1];
        return valid;
    }


    #endif // WIN32

    bool store(uint32_t gameId, const char* gameName, uint16_t dataSize, void* data) {
        return storeImpl(gameId,gameName, dataSize, data);
    }
    bool restore(uint32_t gameId, uint16_t dataSize, void *result) {
        return restoreImpl(gameId,dataSize,result);
    }
}
