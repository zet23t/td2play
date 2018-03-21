#pragma once
#include <inttypes.h>
namespace FlashStore {
    bool store(uint32_t gameId, const char* gameName, uint16_t dataSize, void* data);
    bool restore(uint32_t gameId, uint16_t dataSize, void *result);
}
