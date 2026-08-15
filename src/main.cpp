#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

#include "Display.h"
#include "Sensor.h"
#include "Buffer.h"
#include "Flash.h"
#include "Encoder.h"
#include "wifi.h"
#include "icons.h"
#include "Debug.h"

struct CycleBuffer temperatureBuff = {};
struct CycleBuffer timeBuff = {};
Encoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);
struct tm timeinfo;
volatile uint8_t view = 0;
uint8_t markerPosition;
volatile int16_t currentTemperature;
volatile int16_t move = 0;

#define EVENT_NEW_TEMPERATURE  (1 << 0)
#define EVENT_TIME_UPDATE      (1 << 1)
#define EVENT_ENCODER          (1 << 2)
#define EVENT_VIEW_CHANGE      (1 << 3)

QueueHandle_t recordQueue;
TaskHandle_t displayTaskHandle = NULL;
TimerHandle_t clockTimer;
SemaphoreHandle_t flashMutex;

void clockTimerCallback(TimerHandle_t timer) {
    xTaskNotify(displayTaskHandle, EVENT_TIME_UPDATE, eSetBits);
}


void sampleMeasure(void *pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  while(1) {
    DEBUG_PRINTLN("Zbieram pomiary do rekordu");
    int16_t temperature = getTemperature();
    struct tm timeInfo;
    getLocalTime(&timeInfo); 
    time_t timeSample = mktime(&timeInfo);

    Record record;
    record.temperature = temperature;
    record.time = timeSample;
    DEBUG_PRINTF("Temperatura: %d, Czas: %d\n", record.temperature, record.time);
    
    currentTemperature = record.temperature;
    addToBuffer(currentTemperature, &temperatureBuff);
    addToBuffer(record.time, &timeBuff);

    xQueueSend(recordQueue, &record, portMAX_DELAY);
    xTaskNotify(displayTaskHandle, EVENT_NEW_TEMPERATURE, eSetBits);

    xTaskDelayUntil(&lastWakeTime, 10000);
  }
}

void flashWrite(void *pvParameters) {
  while(1) {
    Record record;
    xQueueReceive(recordQueue, &record, portMAX_DELAY);

    xSemaphoreTake(flashMutex, portMAX_DELAY);
    int address = getCurrentAddress();
    int sectorAddress = getNextSectorToErase(address);
    if(sectorAddress != -1) {
      clearSector(sectorAddress);
    }
    writeRecord(address, &record);

    xSemaphoreGive(flashMutex);
  }
}

void encoderTask(void *pvParameters) {
  while(1) {
    move = encoder.readRotation();
    if(move != 0) {
      xTaskNotify(displayTaskHandle, EVENT_ENCODER, eSetBits);
    }
    if(encoder.readButton()) {
      view = (view + 1) % 3;
      tft.fillScreen(ST7735_WHITE);
      xTaskNotify(displayTaskHandle, EVENT_VIEW_CHANGE, eSetBits);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}


void displayTask(void *pvParameters) {
  while(1) {
    uint32_t events;
    xTaskNotifyWait(0, UINT32_MAX, &events, portMAX_DELAY);

    switch(view) {
    case MAIN:
      if(events & (EVENT_TIME_UPDATE | EVENT_NEW_TEMPERATURE | EVENT_VIEW_CHANGE)) {
        drawIcons();
        writeTemperature(currentTemperature, 40, 95, 100, 25, 2);
        struct tm timeinfo;
        getLocalTime(&timeinfo);
        writeData(timeinfo, 40, 55, 100, 16, 2);
        writeTime(timeinfo, 40, 15, 100, 16, 2);
      }
      break;

    case PLOT:
      if(events & (EVENT_NEW_TEMPERATURE | EVENT_VIEW_CHANGE)) {
        drawAxis();
        drawPlot(&temperatureBuff);
      }
      if(events & EVENT_ENCODER) {
        drawAxis();
        markerPosition = drawMarker(move);
        struct tm timePlot;
        time_t timeSec = timeBuff.buffer[markerPosition];
        localtime_r(&timeSec, &timePlot);
        writeTemperature(temperatureBuff.buffer[markerPosition], 50, 5, 80, 15, 1);
        writeTime(timePlot, 105, 5, 80, 15, 1);
      }
      break;

    case HISTORY:
      if(events & (EVENT_ENCODER | EVENT_VIEW_CHANGE)) {
        struct tm timeHistory;
        getLocalTime(&timeHistory);
        changeDate(&timeHistory, move);
        drawHistoryTemplate();
        writeData(timeHistory, 35, 8, 98, 16, 2);

        int16_t maxTemperature;
        int16_t minTemperature;
        xSemaphoreTake(flashMutex, portMAX_DELAY);
        uint32_t currentAddress = getCurrentAddress();
        uint16_t index = firstAddressOfDay(timeHistory, currentAddress);
        getDayStatistic(index, currentAddress, &maxTemperature, &minTemperature);
        xSemaphoreGive(flashMutex);
        writeTemperature(maxTemperature, 75, 58, 55, 15, 1);
        writeTemperature(minTemperature, 75, 83, 55, 15, 1);
      }
      break;
    }
  }
}

void setup() {

  Serial.begin(115200);

  encoder.enableISR();

  flashInit();
  if(!isFormatted()) {
    flashFormat();
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi OK");
  configTime(3600, 3600, ntpServer); 

  ds.begin();

  displayInit();

  recordQueue = xQueueCreate(10, sizeof(Record));
  clockTimer = xTimerCreate("Clock Timer", pdMS_TO_TICKS(1000), pdTRUE,NULL, clockTimerCallback);

  xTimerStart(clockTimer, 0);
  flashMutex = xSemaphoreCreateMutex();

  xTaskCreate(sampleMeasure, "Sample Measure", 4096, NULL, 0, NULL);
  xTaskCreate(flashWrite, "Flash write", 4096, NULL, 0, NULL);
  xTaskCreate(encoderTask, "Encoder Task", 1024, NULL, 0, NULL);
  xTaskCreate(displayTask, "Display Task", 4096, NULL, 0, &displayTaskHandle);
}


void loop() {

}
