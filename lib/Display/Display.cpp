#include <stdio.h>
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST77xx.h>
#include <Adafruit_GFX.h>
#include <time.h>

#include "pins.h"
#include "Buffer.h"
#include "Display.h"
#include "icons.h"


Adafruit_ST7735 tft = Adafruit_ST7735(CS_PIN, DC_PIN, RST_PIN);

const int PLOT_STEP_X = PLOT_WIDTH / BUFFER_SIZE;

void displayInit() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_WHITE);
  tft.setRotation(1);
  tft.setTextColor(ST7735_BLACK);
  tft.setTextSize(2);
}

void drawAxis() {
  tft.drawLine(PLOT_START_X, PLOT_START_Y,
               PLOT_START_X + PLOT_WIDTH, PLOT_START_Y,
               ST77XX_BLACK);

  tft.drawLine(PLOT_START_X - 1, PLOT_START_Y,
               PLOT_START_X - 1, PLOT_START_Y - PLOT_HEIGHT,
               ST77XX_BLACK);
}

void clearPlot() {
  tft.fillRect(
    PLOT_START_X,
    PLOT_START_Y - PLOT_HEIGHT,
    PLOT_WIDTH,
    PLOT_HEIGHT,
    ST77XX_WHITE
  );
}

void drawPlot(struct CycleBuffer* buffer) {
  clearPlot();

  for(int i = 1; i < buffer->count; i++) {
    int idx1 = (buffer->head + i - 1) % BUFFER_SIZE;
    int idx2 = (buffer->head + i) % BUFFER_SIZE;

    if(buffer->count != BUFFER_SIZE) {
      idx1 = (i - 1) % BUFFER_SIZE;
      idx2 = (i) % BUFFER_SIZE;
    }
    //printf("  idx1: %d, idx2: %d\n", idx1, idx2);

    int x1 = PLOT_START_X + (i - 1) * PLOT_STEP_X;
    int x2 = PLOT_START_X + i * PLOT_STEP_X;

    int y1 = PLOT_START_Y - buffer->buffer[idx1] * PLOT_STEP_Y;
    int y2 = PLOT_START_Y - buffer->buffer[idx2] * PLOT_STEP_Y;

    //printf("    P1(%d, %d), P2(%d, %d)\n", x1, y1, x2, y2);

    tft.drawLine(x1, y1, x2, y2, ST77XX_RED);
  }
}

void drawIcons() {
  tft.drawBitmap(15, 15, epd_bitmap_clock, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(15, 55, epd_bitmap_calendar, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(15, 95, epd_bitmap_termometer, 16, 16, ST7735_WHITE, ST7735_BLACK);
}

void writeTime(struct tm time) {
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
  tft.fillRect(40, 15, 100, 16, ST7735_WHITE);
  tft.setCursor(40, 15);
  tft.print(buf);
}

void writeData(struct tm time) {
  char bufData[9];
  int year = time.tm_year + 1900 - 2000;
  snprintf(bufData, sizeof(bufData), "%02d/%02d/%02d", time.tm_mday, time.tm_mon + 1, year);
  tft.fillRect(40, 55, 100, 16, ST7735_WHITE);
  tft.setCursor(40, 55);
  tft.print(bufData);
}

void writeTemperature(float value) {
  char buf[9];
  snprintf(buf, sizeof(buf), "%.2f C", value);
  tft.setCursor(40, 95);
  tft.fillRect(40, 95, 100, 25, ST77XX_WHITE);
  tft.print(buf);
}





