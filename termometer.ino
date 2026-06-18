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

void setup() {
  Serial.begin(9600);
  selected = ds.select(address);
  tft.initR(INITR_BLACKTAB); 
  tft.fillScreen(ST77XX_WHITE);
  tft.setRotation(1);
  tft.setCursor(10, 10);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_BLACK);
  tft.println("Temperature:");
}

void loop() {
  if (selected) {
    float temperature = ds.getTempC();
    Serial.println(temperature);
    tft.setCursor(40, 50);
    tft.println(temperature);
  }
  delay(5000);
  tft.fillRect(40, 50, 160, 25, ST77XX_WHITE);
  
}
