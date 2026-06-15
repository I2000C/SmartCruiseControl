#pragma once
#include <cinttypes>
#include <Arduino.h>

uint16_t readADC(gpio_num_t pin, uint8_t samples = 16) {
    uint32_t sum = 0;
    for(int i=0; i<samples; i++) {
        sum += analogRead(pin);
    }
    return (uint16_t) (sum / samples);
}
