#include <DS18B20.h>
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

DS18B20 ds(4);
uint8_t address[] = {0x28, 0x0E, 0x6E, 0x22, 0x00, 0x00, 0x00, 0xC9};
uint8_t selected;

#define BUFFER_SIZE 20
float temperature = 0;
float tempBuffer[BUFFER_SIZE];
int plotStartX = 8;
int plotStartY = 120;
int plotHeight = 100;
int plotWeight = 140;
int stepX = 0;
bool plotFirstStep = true;
float prevY;
int prevX;


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
      tabTemperature[i] = ds.getTempC();
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
  static uint16_t head = 0;
  static uint16_t count = 0;
  buffer[head] = temperature;
  head = (head + 1) % BUFFER_SIZE;
  if(count < BUFFER_SIZE){
    count++;
  }
  else{
    for(int i = 0; i < 10; i++){
      Serial.print("Temperatura z bufora: ");
      Serial.println(buffer[head+i]);
    }
    count = 0;
  }
}

void drawPlot(float temperature){
  int plotStepX = plotWeight/BUFFER_SIZE;
  float plotStepY = 2;
  float currentY = plotStartY - temperature * plotStepY;
  int currentX = plotStartX + plotStepX * stepX;
  if(stepX == BUFFER_SIZE){
    stepX = 0;
    tft.fillRect(plotStartX, plotStartY - plotHeight, plotWeight, plotHeight, ST77XX_WHITE);
    currentX = plotStartX;
    plotFirstStep = true;
  }
  stepX++;
  if(!plotFirstStep){
    tft.drawLine(prevX, prevY, currentX, currentY, ST77XX_RED);
  }
  prevY = currentY;
  prevX = currentX;
  plotFirstStep = false;
}

void drawAxis(){
  tft.drawLine(plotStartX, plotStartY, plotStartX + plotWeight, plotStartY, ST77XX_BLACK);
  tft.drawLine(plotStartX - 1, plotStartY, plotStartX - 1, plotStartY - plotHeight,ST77XX_BLACK);
}

void setup() {
  Serial.begin(9600);
  selected = ds.select(address);
  displayInit();
  drawAxis();
}

void loop() {
  if (selected) {
    temperature = getMeanTemperature();
    //updateTemp(temperature);
    collectTempIntoBuffer(temperature, tempBuffer);
    drawPlot(temperature);
  }
  delay(100);
}
