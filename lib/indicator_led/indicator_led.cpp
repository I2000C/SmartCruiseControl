#include "indicator_led.h"
#include <Arduino.h>

static bool failSafeMode = false;

bool IndicatorLed::init() {
    // Configure PWM output for the cruise indicator LED
    if(ledcSetup(INDICATOR_LED_PWM_CHANNEL, INDICATOR_LED_PWM_FREQ_HZ, INDICATOR_LED_PWM_RESOLUTION_BITS)) {
        ledcAttachPin(INDICATOR_LED_PIN, INDICATOR_LED_PWM_CHANNEL);
        failSafeMode = false;
    } else {
        pinMode(INDICATOR_LED_PIN, OUTPUT);
        failSafeMode = true;
    }
    setState(SystemState::STATE_OFF);
    return !failSafeMode;
}

void IndicatorLed::setState(const SystemState& state) {
    // Set LED brightness according to the current cruise control state
    switch(state) {
        case SystemState::STATE_OFF:
            if(failSafeMode) {
                digitalWrite(INDICATOR_LED_PIN, LOW);
            } else {
                ledcWrite(INDICATOR_LED_PWM_CHANNEL, 0);
            }
            break;

        case SystemState::STATE_ACTIVE:
            if(failSafeMode) {
                digitalWrite(INDICATOR_LED_PIN, HIGH);
            } else {
                ledcWrite(INDICATOR_LED_PWM_CHANNEL, INDICATOR_LED_PWM_MAX);
            }
            break;

        case SystemState::STATE_OVERRIDE:
            if(failSafeMode) {
                digitalWrite(INDICATOR_LED_PIN, HIGH);
            } else {
                ledcWrite(INDICATOR_LED_PWM_CHANNEL, INDICATOR_LED_PWM_MAX / 2);
            }
            break;
    }
}
