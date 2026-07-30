#pragma once

#include <stdio.h>
#include <esp_attr.h>

class Encoder {
public:
    Encoder(uint8_t clk, uint8_t dt, uint8_t sw);
    
//-------- POLLING ----------------------------------------    
    int rotationPolling();
    bool buttonPolling();

//---------- ISR ------------------------------------------
    void enableISR();
    void disableISR();
    int8_t readRotation();
    bool readButton();

//---------------------------------------------------------
    int getPosition();
    void resetPosition();

private:
    uint8_t _clk;
    uint8_t _dt;
    uint8_t _sw;
    static Encoder* instance;

    uint8_t _lastCLK;
    uint8_t _lastSW;
    unsigned long _lastTime;
    volatile int _position = 0; 
    volatile int8_t _dir = 0;
    volatile bool _buttonPressed = false;
    volatile int _rotationCounter = 0;
    volatile uint8_t _lastState = 0;

    static void IRAM_ATTR rotationISR();
    static void IRAM_ATTR buttonISR();
};