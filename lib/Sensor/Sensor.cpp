#include <Arduino.h>
#include "Sensor.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include "pins.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds(&oneWire);

float getMeanValue(int amount) {
  float value = 0;
  float tab[amount];
  for (int i = 0; i < amount; i++) {
    ds.requestTemperatures();
    tab[i] = ds.getTempCByIndex(0);
    //Serial.println(tab[i]);
    value += tab[i];
    delay(50);
  }
  value = value / amount;
  //Serial.println("Mean value: " + String(temperature));
  return value;
}