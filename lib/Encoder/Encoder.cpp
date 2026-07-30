#include "Encoder.h"
#include <Arduino.h>

Encoder::Encoder(uint8_t clk, uint8_t dt, uint8_t sw) {
    _clk = clk;
    _dt = dt;
    _sw = sw;

    instance = this;

    pinMode(_clk, INPUT_PULLUP);
    pinMode(_dt, INPUT_PULLUP);
    pinMode(_sw, INPUT_PULLUP);

    _lastCLK = digitalRead(_clk);
    _lastSW = digitalRead(_sw);
    _lastTime = 0;
    _lastState = (digitalRead(_clk) << 1) | digitalRead(_dt);
}

Encoder* Encoder::instance = nullptr;

int Encoder::rotationPolling() {
    uint8_t currentCLK = digitalRead(_clk);
    int value = 0;
    if(currentCLK != _lastCLK) {
        if(digitalRead(_dt) != currentCLK) {
            _position++;
            value = 1;
        }
        else {
            _position--;
            value = -1;
        }
    }
    _lastCLK = currentCLK;
    return value;
}

bool Encoder::buttonPolling() {
    bool state = false;
    if(digitalRead(_sw) != _lastSW) {
        if(millis() - _lastTime > 100){
            state = true;
        }
        else {
            state = false;
        }
        _lastTime = millis();
    }
    return state;
}

int Encoder::getPosition() {
    int position = 0;
    noInterrupts();
    position = _position/2;
    interrupts();
    return position;
}

void Encoder::resetPosition() {
    _position = 0;
}

void IRAM_ATTR Encoder::rotationISR(){
    uint8_t currentState = (digitalRead(instance->_clk) << 1) | digitalRead(instance->_dt);
    uint8_t transition = (instance->_lastState << 2) | currentState;

    static const int8_t table[16] =
    {
         0, -1,  1,  0,
         1,  0,  0, -1,
        -1,  0,  0,  1,
         0,  1, -1,  0
    };


    int8_t movement = table[transition];

    instance->_rotationCounter += movement;
    instance->_position += movement;
    instance->_lastState = currentState;
}

void IRAM_ATTR Encoder::buttonISR() {
    instance->_buttonPressed = true;
}

void Encoder::enableISR() {
    attachInterrupt(digitalPinToInterrupt(_clk), rotationISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_dt),  rotationISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_sw), buttonISR, FALLING);
}

void Encoder::disableISR() {
    detachInterrupt(digitalPinToInterrupt(_clk));
    detachInterrupt(digitalPinToInterrupt(_dt));
    detachInterrupt(digitalPinToInterrupt(_sw));
}

int8_t Encoder::readRotation() {
    int8_t result = 0;
    noInterrupts();
    result = _rotationCounter/2;
    _rotationCounter = 0;
    interrupts();
    return result;
}

bool Encoder::readButton() {
    bool result = _buttonPressed;
    _buttonPressed = false;
    return result;
}

