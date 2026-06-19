#pragma once
#include "constants.h"

#define V_REF 3.3f

// APPS (Accelerator Pedal Position Sensor) ranges
#define APPS1_MIN_V 0.9772f
#define APPS1_MAX_V 3.174f
#define APPS2_MIN_V 0.4588f
#define APPS2_MAX_V 1.485f

inline constexpr float APPS1_BASE = APPS1_MIN_V * THROTTLE_PWM_MAX / V_REF;
inline constexpr float APPS2_BASE = APPS2_MIN_V * THROTTLE_PWM_MAX / V_REF;
inline constexpr float APPS1_MULTI = (APPS1_MAX_V * THROTTLE_PWM_MAX / V_REF - APPS1_BASE) / 100.0f;
inline constexpr float APPS2_MULTI = (APPS2_MAX_V * THROTTLE_PWM_MAX / V_REF - APPS2_BASE) / 100.0f;

namespace Throttle {
    // Initialize throttle PWM outputs, relay pins, and ADC input state
    bool init();

    // Set throttle value. Value must be between 0% and 100%
    void setGeneratedValue(float value);

    // Read throttle value (between 0% and 100%) from actual pedal (APPS2)
    float readPedalValue();

    // When enabled, the generated throttle value is used instead of the actual pedal value.
    // If brake or clutch are active, this will return false and throttle won't be overrided
    bool enableOverride(bool enabled);

    // Returns true if throttle override is active
    bool isOverrideActive();

    // Checks if relay is active or not.
    // This could be used to detect if brake or clutch are active while throttle is being overrided.
    bool checkRelayState();
}
