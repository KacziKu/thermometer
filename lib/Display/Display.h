#pragma once

#include <Adafruit_ST7735.h>

#define PLOT_START_X 8
#define PLOT_0 90
#define PLOT_START_Y 120
#define PLOT_HEIGHT 100
#define PLOT_WIDTH 144
#define PLOT_STEP_Y 2


enum viewTemplate {
  MAIN,
  PLOT,
  HISTORY
};

extern Adafruit_ST7735 tft;

void displayInit();
void drawAxis();
void drawPlot(struct CycleBuffer* buffer);
void clearPlot();
void drawIcons();
void writeTime(struct tm time, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t size);
void writeData(struct tm time, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t size);
void writeTemperature(int16_t value, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t size);
uint8_t drawMarker(int16_t move);
void drawHistoryTemplate();