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
  static uint16_t sample = 0;
  buffer[sample++] = temperature;
  if(sample >= BUFFER_SIZE){
    for(int i = 0; i < 10; i++){
      Serial.println("Temperatura z bufora: ");
      Serial.println(buffer[i]);
      delay(100);
    }
    sample = 0;  
  }
}

void setup() {
  Serial.begin(9600);
  selected = ds.select(address);
  displayInit();
}

void loop() {
  if (selected) {

    //zebranie uśrednionej temperatury z 5 pomiarów co sekundę
    temperature = getMeanTemperature();
    updateTemp(temperature);
    collectTempIntoBuffer(temperature, tempBuffer);
  }
  delay(100);
}
