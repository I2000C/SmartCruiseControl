#pragma once
#include "constants.h"

namespace BrakeClutch {
    // Initialize brake and clutch pins
    inline void init() {
        pinMode(BRAKE_PIN, INPUT_PULLUP);
        pinMode(CLUTCH_PIN, INPUT_PULLUP);
    }

    // Checks if brake pedal is being pressed
    inline bool isBrakePressed() {
        return digitalRead(BRAKE_PIN) == LOW;
    }

    // Checks if clutch pedal is being pressed
    inline bool isClutchPressed() {
        return digitalRead(CLUTCH_PIN) == LOW;
    }
}
