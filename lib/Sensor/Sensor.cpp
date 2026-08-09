#include <Arduino.h>
#include "Sensor.h"
#include <DallasTemperature.h>
#include <OneWire.h>
#include "pins.h"
#include "Debug.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds(&oneWire);

int16_t getTemperature() {
  uint32_t start = micros();

  int16_t result = 0;
  ds.requestTemperatures();
  float value = ds.getTempCByIndex(0);

  uint32_t end = micros();
  DEBUG_PRINTF("Function time getMeanValue: %lu us\n", end - start);

  return (int16_t)(value * 100);
}