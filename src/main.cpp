#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "esp_timer.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "Display.h"
#include "Sensor.h"
#include "Buffer.h"
#include "Flash.h"
#include "Encoder.h"
#include "wifi.h"
#include "icons.h"

struct CycleBuffer buff = {};
Encoder encoder(ENCODER_CLK, ENCODER_DT, ENCODER_SW);
struct tm timeinfo;
uint8_t view = 0;
uint8_t markerPosition;
float temperature = 28;
int16_t move = 0;

QueueHandle_t recordQueue;


void sampleMeasure(void *pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  while(1) {
    float temperature = getMeanValue(3);
    struct tm timeInfo;
    getLocalTime(&timeInfo); 
    time_t timeSample = mktime(&timeinfo);

    Record record;
    record.temperature = temperature;
    record.time = timeSample;
    
    xQueueSend(recordQueue, &record, portMAX_DELAY);
    xTaskDelayUntil(&lastWakeTime, 3000000);
  }
}

void flashWrite(void *pvParameters) {
  while(1) {
    Record record;
    xQueueReceive(recordQueue, &record, portMAX_DELAY);

    int address = getCurrentAddress();
    int sectorAddress = getNextSectorToErase(address);
    clearSector(sectorAddress);
    writeRecord(address, &record);
  }
}

void encoderTask(void *pvParameters) {
  while(1) {
    if(encoder.readButton()) {
      view = (view + 1) % 3;
      tft.fillScreen(ST7735_WHITE);
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void displayTask(void *pvParameters) {
  while(1) {
    switch(view) {
    case MAIN:
      drawIcons();
      writeTemperature(temperature, 40, 95, 100, 25, 2);
      writeData(timeinfo, 40, 55, 100, 16, 2);
      writeTime(timeinfo, 40, 15, 100, 16, 2);
      break;
    case PLOT:
      drawAxis();
      drawPlot(&buff);
      move = encoder.readRotation();
      markerPosition = drawMarker(move);
      //printf("Marker position: %d\n", markerPosition);
      writeTemperature(buff.buffer[markerPosition], 50, 5, 80, 15, 1);
      writeTime(timeinfo, 105, 5, 80, 15, 1);
      break;
    case HISTORY:
      drawHistoryTemplate();
      writeData(timeinfo, 35, 8, 98, 16, 2);
      writeTemperature(temperature, 75, 58, 55, 15, 1);
      writeTemperature(temperature, 75, 83, 55, 15, 1);
      break;
    }
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

void setup() {

  Serial.begin(115200);

  encoder.enableISR();

  flashInit();
  if(isFormatted()) {
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
  xTaskCreate(sampleMeasure, "Sample Measure", 4096, NULL, 0, NULL);
  xTaskCreate(flashWrite, "Flash write", 4096, NULL, 0, NULL);
  xTaskCreate(encoderTask, "Encoder Task", 1024, NULL, 0, NULL);
  xTaskCreate(displayTask, "Display Task", 4096, NULL, 0, NULL);
}


void loop() {

}
