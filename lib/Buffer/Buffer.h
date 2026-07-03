#pragma once

#include <cstdint>

#define BUFFER_SIZE 20

struct CycleBuffer {
    uint16_t head;
    uint16_t count;
    float buffer[BUFFER_SIZE];
};

void addToBuffer(float value, struct CycleBuffer* buff);

void logBuff(struct CycleBuffer* buff);