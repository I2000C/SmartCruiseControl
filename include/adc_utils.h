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

    /**
    * Performs a trimmed mean ADC reading.
    *
    * Acquires multiple ADC samples, sorts them, removes the lowest and highest
    * values, and averages the remaining samples. This approach provides better
    * immunity to random noise and sporadic ADC spikes than a simple average
    */
    inline uint16_t readFiltered(gpio_num_t pin, uint8_t samples = 16, uint8_t samplesToDiscard = 2) {
        uint16_t adcSamples[samples];

        if(samplesToDiscard * 2 >= samples) {
            samplesToDiscard = 0;
        }

        // Adquire ADC samples
        for(int i=0; i<samples; i++) {
            adcSamples[i] = ADC::read(pin);
        }

        // Sort samples using insertion sort
        for(int index=1; index<samples; index++) {
            uint16_t currentSample = adcSamples[index];
            int8_t prevIndex = index - 1;

            while(prevIndex >= 0 && adcSamples[prevIndex] > currentSample) {
                adcSamples[prevIndex + 1] = adcSamples[prevIndex];
                prevIndex--;
            }

            adcSamples[prevIndex + 1] = currentSample;
        }

        // Compute the average after discarding extreme values
        uint32_t sum = 0;
        for(uint8_t i=samplesToDiscard; i<samples-samplesToDiscard; i++) {
            sum += adcSamples[i];
        }

        uint8_t validSamples = samples - (2 * samplesToDiscard);
        return sum / validSamples;
    }

    // Read ADC and apply exponential moving average smoothing
    inline float readWithEMA(gpio_num_t pin, float prevValue, float alpha = 0.1f) {
        uint16_t raw = ADC::read(pin);
        return prevValue + alpha * (raw - prevValue);
    }
}
