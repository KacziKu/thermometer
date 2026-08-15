#include "Flash.h"
#include "Debug.h"

const esp_partition_t *part = nullptr;

void flashInit() {
    part = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA, (esp_partition_subtype_t)0x40, "storage");

    if (part == nullptr)
    {
        DEBUG_PRINTLN("Brak partycji!");
        return;
    }
    DEBUG_PRINTLN("Partition initialized");
}

void flashFormat() {
    FlashHeader header;
    if(part == nullptr) {
        DEBUG_PRINTLN("Flash not initialized");
        return;
    }
    esp_err_t err = esp_partition_erase_range(part, 0, part->size);
    if (err != ESP_OK) {
        DEBUG_PRINTF("Erase error: %d\n", err);
        return;
    }
    header.magic = FLASH_MAGIC;
    err = esp_partition_write(part, HEADER_OFFSET, &header, HEADER_SIZE);
    if (err != ESP_OK) {
        DEBUG_PRINTF("Write error: %d\n", err);
        return;
    }
    DEBUG_PRINTLN("Flash format");
}

bool isFormatted() {
    bool isFormatted = true;
    if(part == nullptr) {
        DEBUG_PRINTLN("Flash not initialized");
        return false;
    }
    FlashHeader header;
    esp_err_t err = esp_partition_read(part, HEADER_OFFSET, &header, HEADER_SIZE);
    if (err != ESP_OK) {
        DEBUG_PRINTF("Read error: %d\n", err);
        return false;
    }
    if(header.magic != FLASH_MAGIC) {
        isFormatted = false;
    }
    return isFormatted;
}

int getCurrentAddress() {
    Record record;
    esp_err_t err;
    for(int sectorOffset = 0x1000; sectorOffset < part->size; sectorOffset += SECTOR_SIZE) {
        DEBUG_PRINTF("Sprawdzam sektor: 0x%x\n", sectorOffset);

        err = esp_partition_read(part, sectorOffset, &record, sizeof(Record));
        if (err != ESP_OK) {
            DEBUG_PRINTF("Read error: %d\n", err);
            return -1;
        }

        if(record.time == EMPTY_RECORD) {
            DEBUG_PRINTLN("Znalazło pusty sektor");
            //sprawdzamy ostatni rekord z wcześniejszego sektoru
            err = esp_partition_read(part, sectorOffset - sizeof(Record), &record, sizeof(Record));
            if (err != ESP_OK) {
                DEBUG_PRINTF("Read error: %d\n", err);
                return -1;
            }

            //znalazło w poprzednim sektorze
            if(record.time == EMPTY_RECORD && sectorOffset != 0x1000) {
                DEBUG_PRINTLN("Znalazło w poprzednim sektorze");
                //przeszukujemy od początku wcześniejszy sektor
                for(int j = sectorOffset - SECTOR_SIZE; j < sectorOffset; j += sizeof(Record)) {
                    err = esp_partition_read(part, j, &record, sizeof(record));
                    if (err != ESP_OK) {
                        DEBUG_PRINTF("Read error: %d\n", err);
                        return -1;
                    }
                    if(record.time == EMPTY_RECORD) {
                        DEBUG_PRINTF("Pusty rekord %x\n", j);
                        return j;
                    }
                }
            }
            else {
                DEBUG_PRINTLN("Nie znalazło w poprzednim sektorze");
                DEBUG_PRINTF("Pusty rekord: %x\n", sectorOffset);
                return sectorOffset;
            }
        }
    }
    return -1;
}

void writeRecord(uint32_t address, Record *record) {
    esp_err_t err = esp_partition_write(part, address, record, sizeof(Record));
    if (err != ESP_OK) {
        DEBUG_PRINTF("Write error: %d\n", err);
        return;
    }

    DEBUG_PRINTF(
        "Zapisano: {temp: %d, time: %d}; Address: %x\n",
        record->temperature,
        record->time,
        address
    );
}

int getNextSectorToErase(uint32_t address) {
    if(address % SECTOR_SIZE != 0) {
        return -1;
    }
    uint32_t adres = ((address % SECTOR_COUNT) << 12) + SECTOR_SIZE;
    DEBUG_PRINTF("Sektor do wyczyszczenia %x\n", adres);
    return adres;
}

void clearSector(uint32_t address) {
    esp_err_t err = esp_partition_erase_range(part, address, SECTOR_SIZE);
    if (err != ESP_OK) {
        DEBUG_PRINTF("Erase error: %d\n", err);
        return;
    }
    DEBUG_PRINTF("Wyczyszczono sektor %x\n", address);
}

void findPartition() {
    if (part == nullptr) {
        DEBUG_PRINTLN("Nie znaleziono partycji!");
        return;
    }

    DEBUG_PRINTLN("Znaleziono partycję!");
    DEBUG_PRINTF("Label   : %s\n", part->label);
    DEBUG_PRINTF("Address : 0x%08X\n", part->address);
    DEBUG_PRINTF("Size    : %u B\n", part->size);
}

void readRecord(uint32_t address, Record *record) {
    esp_err_t err = esp_partition_read(part, address, record, sizeof(Record));
    if (err != ESP_OK) {
        DEBUG_PRINTF("Read error: %d\n", err);
        return;
    }
    DEBUG_PRINTF("Temperature: %d\n", record->temperature);
    DEBUG_PRINTF("Time: %d\n", record->time);
}

uint16_t recordCount(uint32_t currentAddress) {
    uint16_t count = 0;
    Record record;
    readRecord(LAST_ADDRESS, &record);
    //pierwsze zapełnienie pamięci, albo sektor 15 jest wyczyszczony
    if(record.time == EMPTY_RECORD) {
      if(currentAddress == FIRST_ADDRESS) {
        count = 0;
      }
      else {
        count = (currentAddress - FIRST_ADDRESS)/sizeof(Record);
      }
    }
    //pamięć w pełni zapełniona i jest kołowo
    else {
        uint8_t currentSektor = currentAddress/SECTOR_SIZE;
        uint16_t NumberOfRecordsInSector = SECTOR_SIZE/sizeof(Record);
        count = (currentSektor - 1) * NumberOfRecordsInSector + 
                    ((currentAddress - (currentSektor * SECTOR_SIZE))/sizeof(Record));
    }
    DEBUG_PRINTF("Liczba zapisanych rekordów: %d\n", count);
    return count;
}

uint32_t getAddressFromIndex(uint32_t index, uint32_t currentAddress) {

    uint32_t bufferSize = LAST_ADDRESS - FIRST_ADDRESS + sizeof(Record);

    uint32_t currentSector = (currentAddress / SECTOR_SIZE) * SECTOR_SIZE;
    uint32_t oldestAddress = currentSector + 2 * SECTOR_SIZE;

    if(oldestAddress > LAST_ADDRESS) {
        oldestAddress -= bufferSize;
    }

    Record record;
    readRecord(oldestAddress, &record);
    if(record.time == EMPTY_RECORD) {
        return FIRST_ADDRESS;
    }

    uint32_t address = oldestAddress + index * sizeof(Record);
    if(address > LAST_ADDRESS) {
        address -= bufferSize;
    }
    return address;
}

uint16_t firstAddressOfDay(struct tm data, uint32_t currentAddress) {

    struct tm startDay;
    startDay = data;
    startDay.tm_hour = 0;
    startDay.tm_min = 0;
    startDay.tm_sec = 0;
    time_t start = mktime(&startDay);

    uint16_t bufferSize = recordCount(currentAddress);
    uint16_t right = bufferSize;
    uint16_t left = 0;

    while(left < right) {
        uint32_t mid = left + (right - left) / 2;
        
        Record record;
        uint32_t address = getAddressFromIndex(mid, currentAddress);
        readRecord(address, &record);

        if(record.time < start) {
            left = mid + 1;
        }
        else {
            right = mid;
        }
    }
    return left;
}

void getDayStatistic(uint16_t index, uint32_t currentAddress, int16_t *maximum, int16_t *minimum) {
    int16_t min = -5000;
    int16_t max = -5000;
    Record record;

    uint16_t sampleInDay = 288;

    for(int i = 0; i < sampleInDay; i++) {

        uint32_t address = getAddressFromIndex(index + i, currentAddress);
        readRecord(address, &record);

        if(record.temperature > max) {
            max = record.temperature;
        }
        if(record.temperature < min) {
            min = record.temperature;
        }
    }
    *maximum = max;
    *minimum = min;
}
