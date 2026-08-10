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

uint16_t recordCount() {
    uint32_t address = getCurrentAddress();
    uint16_t count = 0;
    Record record;
    readRecord(LAST_ADDRESS, &record);
    //pierwsze zapełnienie pamięci, albo sektor 15 jest wyczyszczony
    if(record.time == EMPTY_RECORD) {
      if(address == FIRST_ADDRESS) {
        count = 0;
      }
      else {
        count = (address - FIRST_ADDRESS)/sizeof(Record);
      }
    }
    //pamięć w pełni zapełniona i jest kołowo
    else {
        uint8_t currentSektor = address/SECTOR_SIZE;
        uint16_t NumberOfRecordsInSector = SECTOR_SIZE/sizeof(Record);
        count = (currentSektor - 1) * NumberOfRecordsInSector + 
                    ((address - (currentSektor * SECTOR_SIZE))/sizeof(Record));
    }
    DEBUG_PRINTF("Liczba zapisanych rekordów: %d\n", count);
    return count;
}

void getDayStatistic(struct tm data, int16_t *maximum, int16_t *minimum) {
    int16_t min = -50;
    int16_t max = -50;

    struct tm startDay;
    startDay = data;
    startDay.tm_hour = 0;
    startDay.tm_min = 0;
    startDay.tm_sec = 0;
    time_t start = mktime(&startDay);

    struct tm endDay;
    endDay = startDay;
    time_t end = mktime(&endDay);
    uint32_t SecInDay = 3600 * 24;
    end -= SecInDay;

    Record record;
    for(int i = FIRST_ADDRESS; i <= LAST_ADDRESS; i += sizeof(Record)) {
       readRecord(i, &record);
       if(record.time >= start && record.time <= end) {
            if(max < record.temperature) {
                max = record.temperature;
            }
            if(min > record.temperature) {
                min = record.temperature;
            }
       }
       *maximum = max;
       *minimum = min;
    }



}
