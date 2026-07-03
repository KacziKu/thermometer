#pragma once
#include "pins.h"
#include <OneWire.h>
#include <DallasTemperature.h>

extern OneWire oneWire;
extern DallasTemperature ds;

float getMeanValue(int amount);
