#pragma once

#include <Arduino.h>
#include <esp_partition.h>

#define SECTOR_COUNT 15
#define SECTOR_SIZE 0x1000
#define FIRST_ADDRESS 0x1000
#define EMPTY_RECORD 0xFFFFFFFF
#define LAST_ADDRESS 0xFFF8

struct Record {
    uint32_t time;
    int16_t temperature;
};

struct FlashHeader {
    uint32_t magic;
};

constexpr uint32_t FLASH_MAGIC = 0xDEADBEEF;
constexpr size_t HEADER_OFFSET = 0;
constexpr size_t HEADER_SIZE = sizeof(FlashHeader);
extern const esp_partition_t *part;

void flashInit();
void flashFormat();
bool isFormatted();
int getCurrentAddress();
void writeRecord(uint32_t address, Record *record);
int getNextSectorToErase(uint32_t address);
void clearSector(uint32_t address);
void findPartition();
void readRecord(uint32_t address, Record *record);
uint16_t recordCount();
void getDayStatistic(struct tm data, int16_t *maximum, int16_t *minimum);

