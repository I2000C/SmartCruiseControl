#pragma once
#include "constants.h"
#include "adc_utils.h"
#include <cinttypes>

#define V_REF 3.3f

// APPS (Accelerator Pedal Position Sensor) ranges
#define APPS1_MIN_V 0.9772f
#define APPS1_MAX_V 3.174f
#define APPS2_MIN_V 0.4588f
#define APPS2_MAX_V 1.485f

static const float APPS1_BASE = APPS1_MIN_V * THROTTLE_PWM_MAX / V_REF;
static const float APPS2_BASE = APPS2_MIN_V * THROTTLE_PWM_MAX / V_REF;
static const float APPS1_MULTI = (APPS1_MAX_V * THROTTLE_PWM_MAX / V_REF - APPS1_BASE) / 100.0f;
static const float APPS2_MULTI = (APPS2_MAX_V * THROTTLE_PWM_MAX / V_REF - APPS2_BASE) / 100.0f;

namespace Throttle {
    void init();

    // Set throttle value. Value must be between 0% and 100%
    void setGeneratedValue(float value);

    // Read throttle value (between 0% and 100%) from actual pedal (APPS2)
    float readPedalValue();

    // When enabled, the generated throttle value is used instead of the actual pedal value.
    void enableOverride(bool enabled);
}

void Throttle::init() {
    pinMode(THROTTLE_RELAY_PIN, OUTPUT);
    enableOverride(false);

    ledcSetup(THROTTLE_APPS1_PWM_CHANNEL, THROTTLE_PWM_FREQ_HZ, THROTTLE_PWM_RESOLUTION_BITS);
    ledcSetup(THROTTLE_APPS2_PWM_CHANNEL, THROTTLE_PWM_FREQ_HZ, THROTTLE_PWM_RESOLUTION_BITS);
    ledcAttachPin(THROTTLE_APPS1_PIN, THROTTLE_APPS1_PWM_CHANNEL);
    ledcAttachPin(THROTTLE_APPS2_PIN, THROTTLE_APPS2_PWM_CHANNEL);
    setGeneratedValue(0.0f);

    analogReadResolution(THROTTLE_PWM_RESOLUTION_BITS);
}

void Throttle::setGeneratedValue(float value) {
    value = constrain(value, 0.0f, 100.0f);

    uint16_t apps1Value = min(THROTTLE_PWM_MAX, (int) round(APPS1_BASE + value * APPS1_MULTI));
    uint16_t apps2Value = min(THROTTLE_PWM_MAX, (int) round(APPS2_BASE + value * APPS2_MULTI));

    ledcWrite(THROTTLE_APPS1_PWM_CHANNEL, apps1Value);
    ledcWrite(THROTTLE_APPS2_PWM_CHANNEL, apps2Value);
}

float Throttle::readPedalValue() {
    uint16_t rawValue = readADC(THROTTLE_APPS2_PEDAL_PIN);
    float apps2Value = (rawValue - APPS2_BASE) / APPS2_MULTI;
    apps2Value = constrain(apps2Value, 0.0f, 100.0f);
    return apps2Value;
}

void Throttle::enableOverride(bool enabled) {
    digitalWrite(THROTTLE_RELAY_PIN, enabled);
}
