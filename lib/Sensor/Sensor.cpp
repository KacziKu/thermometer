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
    value += tab[i];
  }
  value = value / amount;
  return value;
}