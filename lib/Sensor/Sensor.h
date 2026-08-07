#pragma once
#include "pins.h"
#include <OneWire.h>
#include <DallasTemperature.h>

extern OneWire oneWire;
extern DallasTemperature ds;

int16_t getMeanValue(int amount);
