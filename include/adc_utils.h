#pragma once
#include <cinttypes>
#include <Arduino.h>

namespace ADC {
    // Read one raw ADC sample from the specified pin
    inline uint16_t read(gpio_num_t pin) {
        return analogRead(pin);
    }

    // Read multiple ADC samples and return the average value
    inline uint16_t readWithSamples(gpio_num_t pin, uint8_t samples = 8) {
        uint32_t sum = 0;
        for(uint8_t i=0; i<samples; i++) {
            sum += ADC::read(pin);
        }
        return (uint16_t) (sum / samples);
    }

    // Read ADC and apply exponential moving average smoothing
    inline float readWithEMA(gpio_num_t pin, float prevValue, float alpha = 0.1f) {
        uint16_t raw = ADC::read(pin);
        return prevValue + alpha * (raw - prevValue);
    }
}
