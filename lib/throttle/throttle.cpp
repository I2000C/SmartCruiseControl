#include "throttle.h"
#include "adc_utils.h"
#include <cinttypes>

bool Throttle::init() {
    // Initialize relay and throttle PWM outputs
    pinMode(THROTTLE_RELAY_PIN, OUTPUT);
    enableOverride(false);

    if(!ledcSetup(THROTTLE_APPS1_PWM_CHANNEL, THROTTLE_PWM_FREQ_HZ, THROTTLE_PWM_RESOLUTION_BITS)) {
        return false;
    }

    if(!ledcSetup(THROTTLE_APPS2_PWM_CHANNEL, THROTTLE_PWM_FREQ_HZ, THROTTLE_PWM_RESOLUTION_BITS)) {
        return false;
    }
    
    ledcAttachPin(THROTTLE_APPS1_PIN, THROTTLE_APPS1_PWM_CHANNEL);
    ledcAttachPin(THROTTLE_APPS2_PIN, THROTTLE_APPS2_PWM_CHANNEL);
    setGeneratedValue(0.0f);

    analogReadResolution(THROTTLE_PWM_RESOLUTION_BITS);
    return true;
}

void Throttle::setGeneratedValue(float value) {
    value = constrain(value, 0.0f, 100.0f);

    uint16_t apps1Value = min(THROTTLE_PWM_MAX, (int) round(APPS1_BASE + value * APPS1_MULTI));
    uint16_t apps2Value = min(THROTTLE_PWM_MAX, (int) round(APPS2_BASE + value * APPS2_MULTI));

    ledcWrite(THROTTLE_APPS1_PWM_CHANNEL, apps1Value);
    ledcWrite(THROTTLE_APPS2_PWM_CHANNEL, apps2Value);
}

float Throttle::readPedalValue() {
    float value = ADC::readFiltered(THROTTLE_APPS2_PEDAL_PIN);
    float apps2Value = (value - APPS2_BASE) / APPS2_MULTI;
    apps2Value = constrain(apps2Value, 0.0f, 100.0f);
    return apps2Value;
}

void Throttle::enableOverride(bool enabled) {
    // Engage or disengage throttle override relay
    digitalWrite(THROTTLE_RELAY_PIN, enabled);
}
