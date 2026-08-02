#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

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

void setup() {
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
}


void loop() {

  float temperature = getMeanValue(5);
  addToBuffer(temperature, &buff);

  if(encoder.readButton()) {
    view = (view + 1) % 3;
    tft.fillScreen(ST7735_WHITE);
  }

  int16_t move = encoder.readRotation();

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
      drawMarker(move);
      writeTemperature(temperature, 50, 5, 80, 20, 1);
      writeTime(timeinfo, 105, 5, 80, 20, 1);
      break;
    case HISTORY:
      drawHistoryTemplate();
      writeData(timeinfo, 35, 8, 100, 16, 2);
      writeTemperature(temperature, 75, 58, 55, 15, 1);
      writeTemperature(temperature, 75, 83, 55, 15, 1);
      break;
  }
}
