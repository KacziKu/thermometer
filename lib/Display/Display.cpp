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
#include "Debug.h"


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
  //pozioma
  tft.drawLine(PLOT_START_X, PLOT_0,
               PLOT_START_X + PLOT_WIDTH, PLOT_0,
               ST77XX_BLACK);

  //pionowa
  tft.drawLine(PLOT_START_X - 1, PLOT_START_Y,
               PLOT_START_X - 1, PLOT_START_Y - PLOT_HEIGHT,
               ST77XX_BLACK);

  tft.drawBitmap(PLOT_START_X + PLOT_WIDTH + 1, PLOT_0 - 2,
                  epd_bitmap_arrow_plot_left,
                  5,
                  5,
                  ST7735_WHITE, ST7735_BLACK);

  tft.drawBitmap(PLOT_START_X - 3, PLOT_0 - 75,
                  epd_bitmap_arrow_plot_up,
                  5,
                  5,
                  ST7735_WHITE, ST7735_BLACK);
}

void clearPlot() {
  tft.fillRect(
    PLOT_START_X,
    PLOT_START_Y - PLOT_HEIGHT,
    PLOT_WIDTH + 1,
    70,
    ST77XX_WHITE
  );

    tft.fillRect(
    PLOT_START_X,
    PLOT_0 + 1,
    PLOT_WIDTH + 1,
    30,
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
    DEBUG_PRINTF("  idx1: %d, idx2: %d\n", idx1, idx2);

    int x1 = PLOT_START_X + (i - 1) * PLOT_STEP_X;
    int x2 = PLOT_START_X + i * PLOT_STEP_X;

    int y1 = PLOT_0 - buffer->buffer[idx1]/100.2f * PLOT_STEP_Y;
    int y2 = PLOT_0 - buffer->buffer[idx2]/100.2f * PLOT_STEP_Y;

    DEBUG_PRINTF("    P1(%d, %d), P2(%d, %d)\n", x1, y1, x2, y2);

    tft.drawLine(x1, y1, x2, y2, ST77XX_RED);
  }
}

void drawIcons() {
  tft.drawBitmap(15, 15, epd_bitmap_clock, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(15, 55, epd_bitmap_calendar, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(15, 95, epd_bitmap_termometer, 16, 16, ST7735_WHITE, ST7735_BLACK);
}

void writeTime(struct tm time, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t size) {
  char buf[9];
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", time.tm_hour, time.tm_min, time.tm_sec);
  tft.fillRect(x, y, w, h, ST7735_WHITE);
  tft.setCursor(x, y);
  tft.setTextSize(size);
  tft.print(buf);
}

void writeData(struct tm time, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t size) {
  char bufData[9];
  int year = time.tm_year + 1900 - 2000;
  snprintf(bufData, sizeof(bufData), "%02d/%02d/%02d", time.tm_mday, time.tm_mon + 1, year);
  tft.fillRect(x, y, w, h, ST7735_WHITE);
  tft.setCursor(x, y);
  tft.setTextSize(size);
  tft.print(bufData);
}

void writeTemperature(int16_t value, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t size) {
  float temperature = value/100.2f;
  char buf[9];
  snprintf(buf, sizeof(buf), "%.2f C", temperature);
  tft.setCursor(x, y);
  tft.fillRect(x, y, w, h, ST77XX_WHITE);
  tft.setTextSize(size);
  if(size == 1) {
    tft.drawBitmap(x + 29, y, epd_bitmap_small_deg, 7, 7, ST7735_WHITE, ST7735_BLACK);
  }
  else if(size == 2) {
    tft.drawBitmap(x + 58, y, epd_bitmap_big_deg, 14, 14, ST7735_WHITE, ST7735_BLACK);
  }
  tft.print(buf);
}

uint8_t drawMarker(int16_t move) {
  static uint8_t x;
  tft.drawFastVLine(x, PLOT_START_Y - PLOT_HEIGHT + 1, PLOT_HEIGHT, ST7735_WHITE);
  x += move;
  if(x > PLOT_START_X + PLOT_WIDTH) {
    x = PLOT_START_X + PLOT_WIDTH;
  }
  else if(x < PLOT_START_X) {
    x = PLOT_START_X;
  }
  tft.drawFastVLine(x, PLOT_START_Y - PLOT_HEIGHT + 1, PLOT_HEIGHT, ST7735_BLUE);
  return (x - PLOT_START_X)/PLOT_STEP_X;
}

void drawHistoryTemplate() {
  tft.drawFastVLine(30, 49, 50, ST7735_BLACK);
  tft.drawFastVLine(60, 49, 50, ST7735_BLACK);
  tft.drawFastVLine(130, 49, 50, ST7735_BLACK);
  tft.drawFastHLine(30, 49, 100, ST7735_BLACK);
  tft.drawFastHLine(30, 74, 100, ST7735_BLACK);
  tft.drawFastHLine(30, 99, 100, ST7735_BLACK);
  tft.drawBitmap(37, 54, epd_bitmap_sun, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(37, 78, epd_bitmap_moon, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(13, 8, epd_bitmap_arrow_left, 16, 16, ST7735_WHITE, ST7735_BLACK);
  tft.drawBitmap(133, 8, epd_bitmap_arrow_right, 16, 16, ST7735_WHITE, ST7735_BLACK);
}

void changeDate(struct tm *time, int16_t move) {
  static int8_t x = 0;
  x += move;
  if(x < -5) {
    x = -5;
  }
  else if(x > -1) {
    x = -1;
  }
  time_t timeStamps = mktime(time);
  uint32_t secInDay = 3600 * 24;
  timeStamps = timeStamps + x * secInDay;
  localtime_r(&timeStamps, time);
}



