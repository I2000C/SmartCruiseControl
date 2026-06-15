#pragma once
#include "constants.h"
#include "state.h"
#include <Arduino.h>

namespace IndicatorLed {
    void init();

    void setState(const SystemState& state);
}

void IndicatorLed::init() {
    ledcSetup(INDICATOR_LED_PWM_CHANNEL, INDICATOR_LED_PWM_FREQ_HZ, INDICATOR_LED_PWM_RESOLUTION_BITS);
    ledcAttachPin(INDICATOR_LED_PIN, INDICATOR_LED_PWM_CHANNEL);
    setState(SystemState::STATE_OFF);
}

void IndicatorLed::setState(const SystemState& state) {
    switch(state) {
        case SystemState::STATE_OFF:
            ledcWrite(INDICATOR_LED_PWM_CHANNEL, 0);
            break;

        case SystemState::STATE_ACTIVE:
            ledcWrite(INDICATOR_LED_PWM_CHANNEL, INDICATOR_LED_PWM_MAX);
            break;

        case SystemState::STATE_OVERRIDE:
            ledcWrite(INDICATOR_LED_PWM_CHANNEL, INDICATOR_LED_PWM_MAX / 2);
            break;
    }
}
