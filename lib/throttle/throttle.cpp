#include "throttle.h"
#include "adc_utils.h"
#include <cinttypes>

static bool overrideActive = false; // Tracks whether throttle override is enabled

void Throttle::init() {
    // Initialize relay and throttle PWM outputs
    pinMode(THROTTLE_RELAY_PIN, OUTPUT);
    pinMode(THROTTLE_RELAY_CHECK_PIN, INPUT);
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
    static float previousValue = 0.0f;
    float value = ADC::readWithEMA(THROTTLE_APPS2_PEDAL_PIN, previousValue);
    previousValue = value;
    float apps2Value = (value - APPS2_BASE) / APPS2_MULTI;
    apps2Value = constrain(apps2Value, 0.0f, 100.0f);
    return apps2Value;
}

bool Throttle::enableOverride(bool enabled) {
    // Engage or disengage throttle override relay and verify its state
    overrideActive = enabled;
    digitalWrite(THROTTLE_RELAY_PIN, enabled);
    if(enabled) {
        delayMicroseconds(10);
        bool relayActuallyEnabled = digitalRead(THROTTLE_RELAY_CHECK_PIN);
        if(!relayActuallyEnabled) {
            overrideActive = false;
            digitalWrite(THROTTLE_RELAY_PIN, false);
            return false;
        }
    }
    return true;
}

bool Throttle::isOverrideActive() {
    return overrideActive;
}

bool Throttle::checkRelayState() {
    if(!overrideActive) {
        return false;
    }
    bool relayActuallyEnabled = digitalRead(THROTTLE_RELAY_CHECK_PIN);
    return relayActuallyEnabled;
}
