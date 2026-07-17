//#include <DS18B20.h>
#include <OneWire.h>
#include <SPI.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST77xx.h>
#include <Adafruit_GFX.h>

#define MOSI_PIN 23
#define CLK_PIN  18
#define CS_PIN   27
#define DC_PIN   26
#define RST_PIN  25
#define MISO_PIN 19

Adafruit_ST7735 tft = Adafruit_ST7735(CS_PIN, DC_PIN, RST_PIN);

//DS18B20 ds(4);
uint8_t address[] = {0x28, 0x0E, 0x6E, 0x22, 0x00, 0x00, 0x00, 0xC9};
uint8_t selected;

#define BUFFER_SIZE 20
#define PLOT_START_X 8
#define PLOT_START_Y 120
#define PLOT_HEIGHT 100
#define PLOT_WEIGHT 140
#define PLOT_STEP_Y 2
int plotStepX = PLOT_WEIGHT/BUFFER_SIZE;
int stepX = 0;
bool plotFirstStep = true;
bool bufferFull = false;

struct temperatureHistory{
  float tempBuffer[BUFFER_SIZE];
  uint16_t head;
  uint16_t count;
};

temperatureHistory history = {};

void displayInit(){
  tft.initR(INITR_BLACKTAB); 
  tft.fillScreen(ST77XX_WHITE);
  tft.setRotation(1);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  tft.println("Temperature:");
}

float getMeanTemperature(){
  float temperature = 0;
  float tabTemperature[5];
    for(int i = 0; i < 5; i++){
      //tabTemperature[i] = ds.getTempC();
      Serial.println(tabTemperature[i]);
      temperature += tabTemperature[i];
      delay(100);
    }
  temperature = temperature/5;
  Serial.println("Temperatura uśredniona: " + String(temperature));
  return temperature;
}

void updateTemp(float temperature){
  tft.setCursor(40, 50);
  tft.fillRect(40, 50, 160, 25, ST77XX_WHITE);
  tft.println(temperature);
}

void collectTempIntoBuffer(float temperature, float *buffer){
  buffer[history.head] = temperature;
  history.head = (history.head + 1) % BUFFER_SIZE;
  if(history.count < BUFFER_SIZE){
    history.count++;
  }
}

void drawPlot(float temperature){
  static float prevY;
  static int prevX;
  float currentY = PLOT_START_Y - temperature * PLOT_STEP_Y;
  int currentX = PLOT_START_X + plotStepX * stepX;
  stepX++;
  if(!plotFirstStep){
    if(history.count < BUFFER_SIZE){
      tft.drawLine(prevX, prevY, currentX, currentY, ST77XX_RED);
    }
  }
  prevY = currentY;
  prevX = currentX;
  plotFirstStep = false;
}

void updatePlot(){
  if(history.count >= BUFFER_SIZE){
    tft.fillRect(PLOT_START_X, PLOT_START_Y - PLOT_HEIGHT, PLOT_WEIGHT, PLOT_HEIGHT, ST77XX_WHITE);
    for (int i = 1; i < BUFFER_SIZE; i++) {
      int idx1 = (history.head + i - 1) % BUFFER_SIZE;
      int idx2 = (history.head + i) % BUFFER_SIZE;
      int x1 = PLOT_START_X + (i - 1) * plotStepX;
      int x2 = PLOT_START_X + i * plotStepX;
      int y1 = PLOT_START_Y - history.tempBuffer[idx1] * PLOT_STEP_Y;
      int y2 = PLOT_START_Y - history.tempBuffer[idx2] * PLOT_STEP_Y;
      tft.drawLine(x1, y1, x2, y2, ST77XX_RED);
    }
  }
}

void drawAxis(){
  tft.drawLine(PLOT_START_X, PLOT_START_Y, PLOT_START_X + PLOT_WEIGHT, PLOT_START_Y, ST77XX_BLACK);
  tft.drawLine(PLOT_START_X - 1, PLOT_START_Y, PLOT_START_X - 1, PLOT_START_Y - PLOT_HEIGHT, ST77XX_BLACK);
}

void setup() {
  Serial.begin(9600);
  //selected = ds.select(address);
  displayInit();
  drawAxis();
}

void loop() {
  if (selected) {
    float temperature = getMeanTemperature();
    //updateTemp(temperature);
    collectTempIntoBuffer(temperature, history.tempBuffer);
    drawPlot(temperature);
    updatePlot();
  }
  delay(100);
}
