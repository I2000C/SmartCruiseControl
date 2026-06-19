#pragma once
#include "constants.h"
#include "state.h"

namespace IndicatorLed {
    // Initialize PWM LED hardware
    bool init();

    // Set the indicator brightness based on cruise control state
    void setState(const SystemState& state);
}

