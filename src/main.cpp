#include <Arduino.h>

#include "Display.h"
#include "Sensor.h"
#include "Buffer.h"

struct CycleBuffer buff = {};

void setup() {
  ds.begin();
  displayInit();
  drawAxis();
}

void loop() {
  float temperature = getMeanValue(5);
  addToBuffer(temperature, &buff);
  logBuff(&buff);
  drawPlot(&buff);
  delay(100);
}