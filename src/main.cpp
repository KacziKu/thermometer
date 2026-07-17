#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#include "Display.h"
#include "Sensor.h"
#include "Buffer.h"

struct CycleBuffer buff = {};

const char* ssid = "MAKLAREN44";
const char* password = "maklaren44";
const char* ntpServer = "time.google.com";

 struct tm timeinfo;

void setup() {
  Serial.begin(115200);
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
  //drawAxis();
  drawIcons();
}

void loop() {
  float temperature = getMeanValue(5);
  addToBuffer(temperature, &buff);
  //logBuff(&buff);
  //drawPlot(&buff);
    if(getLocalTime(&timeinfo))
  {
    writeTime(timeinfo);
    writeData(timeinfo);
  }
  writeTemperature(temperature);
  //delay(100);
}



