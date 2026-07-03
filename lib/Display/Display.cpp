#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST77xx.h>
#include <Adafruit_GFX.h>
#include "pins.h"
#include "Buffer.h"
#include "Display.h"



Adafruit_ST7735 tft = Adafruit_ST7735(CS_PIN, DC_PIN, RST_PIN);

const int PLOT_STEP_X = PLOT_WIDTH / BUFFER_SIZE;

void displayInit() {
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_WHITE);
  tft.setRotation(1);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  tft.println("Temperature:");
}

void drawAxis() {
  tft.drawLine(PLOT_START_X, PLOT_START_Y,
               PLOT_START_X + PLOT_WIDTH, PLOT_START_Y,
               ST77XX_BLACK);

  tft.drawLine(PLOT_START_X - 1, PLOT_START_Y,
               PLOT_START_X - 1, PLOT_START_Y - PLOT_HEIGHT,
               ST77XX_BLACK);
}

void updateValue(float value) {
  tft.setCursor(40, 50);
  tft.fillRect(40, 50, 160, 25, ST77XX_WHITE);
  tft.println(value);
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


