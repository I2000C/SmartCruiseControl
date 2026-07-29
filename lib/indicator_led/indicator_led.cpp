#include "indicator_led.h"
#include <Arduino.h>

static bool failSafeMode = false;

// Manage override mode PWM
static constexpr int incrementPwmStep = 20;
static SystemState previousState = SystemState::STATE_OFF;
static int currentPwmValue = 0;
static bool incrementingPwm = false;

bool IndicatorLed::init() {
    // Configure PWM output for the cruise indicator LED
    if(ledcSetup(INDICATOR_LED_PWM_CHANNEL, INDICATOR_LED_PWM_FREQ_HZ, INDICATOR_LED_PWM_RESOLUTION_BITS)) {
        ledcAttachPin(INDICATOR_LED_PIN, INDICATOR_LED_PWM_CHANNEL);
        failSafeMode = false;
    } else {
        pinMode(INDICATOR_LED_PIN, OUTPUT);
        failSafeMode = true;
    }
    if(ledcSetup(INDICATOR_LED_BUILTIN_PWM_CHANNEL, INDICATOR_LED_PWM_FREQ_HZ, INDICATOR_LED_PWM_RESOLUTION_BITS)) {
        ledcAttachPin(INDICATOR_LED_BUILTIN_PIN, INDICATOR_LED_BUILTIN_PWM_CHANNEL);
        failSafeMode = false;
    } else {
        pinMode(INDICATOR_LED_BUILTIN_PIN, OUTPUT);
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
                digitalWrite(INDICATOR_LED_BUILTIN_PIN, LOW);
            } else {
                ledcWrite(INDICATOR_LED_PWM_CHANNEL, 0);
                ledcWrite(INDICATOR_LED_BUILTIN_PWM_CHANNEL, 0);
            }
            break;

        case SystemState::STATE_ACTIVE:
            if(failSafeMode) {
                digitalWrite(INDICATOR_LED_PIN, HIGH);
                digitalWrite(INDICATOR_LED_BUILTIN_PIN, HIGH);
            } else {
                ledcWrite(INDICATOR_LED_PWM_CHANNEL, INDICATOR_LED_PWM_MAX);
                ledcWrite(INDICATOR_LED_BUILTIN_PWM_CHANNEL, INDICATOR_LED_PWM_MAX);
            }
            break;

        case SystemState::STATE_OVERRIDE:
            if(failSafeMode) {
                digitalWrite(INDICATOR_LED_PIN, HIGH);
                digitalWrite(INDICATOR_LED_BUILTIN_PIN, HIGH);
            } else {
                if(previousState != SystemState::STATE_OVERRIDE) {
                    currentPwmValue = 0;
                    incrementingPwm = true;
                }

                ledcWrite(INDICATOR_LED_PWM_CHANNEL, currentPwmValue);
                ledcWrite(INDICATOR_LED_BUILTIN_PWM_CHANNEL, currentPwmValue);

                if(incrementingPwm) {
                    currentPwmValue += incrementPwmStep;
                    if(currentPwmValue > INDICATOR_LED_PWM_MAX) {
                        currentPwmValue = INDICATOR_LED_PWM_MAX;
                        incrementingPwm = false;
                    }
                } else {
                    currentPwmValue -= incrementPwmStep;
                    if(currentPwmValue < 0) {
                        currentPwmValue = 0;
                        incrementingPwm = true;
                    }
                }
            }
            break;
    }

    previousState = state;
}
