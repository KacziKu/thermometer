#pragma once

#include <Adafruit_ST7735.h>

#define PLOT_START_X 8
#define PLOT_START_Y 120
#define PLOT_HEIGHT 100
#define PLOT_WIDTH 140
#define PLOT_STEP_Y 2

extern Adafruit_ST7735 tft;

void displayInit();
void drawAxis();
void drawPlot(struct CycleBuffer* buffer);
void clearPlot();
void drawIcons();
void writeTime(struct tm time);
void writeData(struct tm time);
void writeTemperature(float value);