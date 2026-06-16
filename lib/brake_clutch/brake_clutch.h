#pragma once
#include "constants.h"

namespace BrakeClutch {
    void init();

    bool isPressed();
}

void BrakeClutch::init() {
    pinMode(BRAKE_CLUTCH_PIN, INPUT_PULLUP);
}

bool BrakeClutch::isPressed() {
    return digitalRead(BRAKE_CLUTCH_PIN) == LOW;
}
