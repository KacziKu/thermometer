#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include "esp_timer.h"

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
float temperature = 0;
bool measureFlag = false;
bool displayFlag = false;

esp_timer_handle_t timer_measure = NULL;
esp_timer_handle_t timer_display = NULL;

static void measure(void *args) {
  printf("przerwanie measure\n");
  measureFlag = true;
}

static void display(void *args) {
  printf("przerwanie display\n");
  displayFlag = true;
}

void setup() {
    esp_timer_create_args_t timer_measure_conf = {
      .callback = &measure,
      .name = "measure",
  };

  esp_timer_create_args_t timer_display_conf = {
    .callback = &display,
    .name = "display"
  };

  esp_timer_create(&timer_measure_conf, &timer_measure);
  esp_timer_create(&timer_display_conf, &timer_display);

  Serial.begin(115200);
  encoder.enableISR();
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
  esp_timer_start_periodic(timer_measure, 1000000);
  esp_timer_start_periodic(timer_display, 500000);
}


void loop() {
  if(measureFlag) {
  temperature = getMeanValue(5);
  addToBuffer(temperature, &buff);
  measureFlag = false;
  }

  if(encoder.readButton()) {
    view = (view + 1) % 3;
    tft.fillScreen(ST7735_WHITE);
  }

  int16_t move = encoder.readRotation();

  switch(view) {
    case MAIN:
      drawIcons();
      if(displayFlag) {
        writeTemperature(temperature, 40, 95, 100, 25, 2);
        writeData(timeinfo, 40, 55, 100, 16, 2);
        writeTime(timeinfo, 40, 15, 100, 16, 2);
      }
      break;
    case PLOT:
      drawAxis();
      if(displayFlag) {
        drawPlot(&buff);
        markerPosition = drawMarker(move);
        printf("Marker position: %d\n", markerPosition);
        writeTemperature(buff.buffer[markerPosition], 50, 5, 80, 15, 1);
        writeTime(timeinfo, 105, 5, 80, 15, 1);
      }
      break;
    case HISTORY:
      drawHistoryTemplate();
      if(displayFlag) {
        writeData(timeinfo, 35, 8, 98, 16, 2);
        writeTemperature(temperature, 75, 58, 55, 15, 1);
        writeTemperature(temperature, 75, 83, 55, 15, 1);
      }
      break;
  }
}
