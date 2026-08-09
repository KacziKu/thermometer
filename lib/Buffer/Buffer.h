#pragma once

#include <cstdint>

#define BUFFER_SIZE 144

struct CycleBuffer {
    uint16_t head;
    uint16_t count;
    float buffer[BUFFER_SIZE];
};

void addToBuffer(int16_t value, struct CycleBuffer* buff);

void logBuff(struct CycleBuffer* buff);