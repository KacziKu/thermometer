#include <Arduino.h>
#include "Sensor.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include "pins.h"
#include "Debug.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds(&oneWire);

int16_t getMeanValue(int amount) {
  float value = 0;
  int16_t result = 0;
  for (int i = 0; i < amount; i++) {
    ds.requestTemperatures();
    value += ds.getTempCByIndex(0);
  }
  value = value / amount;
  DEBUG_PRINTF("float temperature: %f\n", value);
  result = value * 100;
  DEBUG_PRINTF("int16_t temperature: %d\n", result);
  return result;
}