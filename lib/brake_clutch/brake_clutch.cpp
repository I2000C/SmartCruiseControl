#include "brake_clutch.h"
#include "constants.h"

void BrakeClutch::init() {
    pinMode(BRAKE_CLUTCH_PIN, INPUT_PULLUP);
}

bool BrakeClutch::isPressed() {
    return digitalRead(BRAKE_CLUTCH_PIN) == LOW;
}
