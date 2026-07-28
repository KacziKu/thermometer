#pragma once

#define DEBUG

#ifdef DEBUG
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
    #define DEBUG_PRINTLN(x) Serial.println(x)

#else
    #define DEBUG_PRINTF(...)
    #define DEBUG_PRINTLN(x)
#endif