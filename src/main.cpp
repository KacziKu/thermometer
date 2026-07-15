#include <Arduino.h>

#include "Display.h"
#include "Sensor.h"
#include "Buffer.h"
#include "icons.h"

struct CycleBuffer buff = {};

void setup() {
  ds.begin();
  displayInit();
  //drawAxis();
  tft.drawBitmap(15, 15, epd_bitmap_termometer, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(45, 15, epd_bitmap_calendar, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(75, 15, epd_bitmap_clock, 16, 16, ST7735_WHITE, ST7735_BLACK);
}

void loop() {
  float temperature = getMeanValue(5);
  addToBuffer(temperature, &buff);
  //logBuff(&buff);
  //drawPlot(&buff);
  delay(100);
}