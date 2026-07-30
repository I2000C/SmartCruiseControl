#include "cruise_control.h"
#include "indicator_led.h"
#include "throttle.h"
#include "brake_clutch.h"
#include "debug.h"

bool CruiseControl::canEnableCruise(bool isResume, const VehicleState& vehicleState) {
    // Check brake and clutch
    if(BrakeClutch::isBrakePressed() || BrakeClutch::isClutchPressed()) {
        return false;
    }

    // Validate vehicle state before enabling or resuming cruise control
    if(!vehicleState.speedValid) {
        return false;
    }

    float currentSpeed = vehicleState.speed;
    if(currentSpeed < MIN_CAR_SPEED_KMH || currentSpeed > MAX_CAR_SPEED_KMH) {
        return false;
    }
    if(isResume) {
        if(targetSpeed < MIN_SPEED_KMH || targetSpeed > MAX_SPEED_KMH) {
            return false;
        }
        if(fabsf(currentSpeed - targetSpeed) > MAX_TARGET_SPEED_DIFF) {
            return false;
        }
    }

    // Validate RPM freshness and range
    uint32_t rpmLastUpdate = vehicleState.rpmLastUpdate;
    if(millis() - rpmLastUpdate > RPM_TIMEOUT_MS) {
        return false;
    }
    uint16_t rpm = vehicleState.rpm;
    if(rpm < MIN_RPM || rpm > MAX_RPM) {
        return false;
    }

    return true;
}

bool CruiseControl::hasButtonEvent(const CCButton button) {
    // Button state changed
    if(button != previousButton) {
        previousButton = button;

        if(button != CCButton::BUTTON_NONE) {
            buttonHoldTicks = 0;
            return true;        // Initial press
        }

        return false;           // Button released
    }

    // No button pressed
    if(button == CCButton::BUTTON_NONE) {
        return false;
    }

    // Button held
    buttonHoldTicks++;

    // Wait for initial delay
    if(buttonHoldTicks < BUTTONS_REPEAT_DELAY_TICKS) {
        return false;
    }

    // Generate repeat events every BUTTONS_REPEAT_PERIOD_TICKS
    return ((buttonHoldTicks - BUTTONS_REPEAT_DELAY_TICKS) % BUTTONS_REPEAT_PERIOD_TICKS) == 0;
}

void CruiseControl::loop(const VehicleState& vehicleState, const CCButton button) {
    // Update indicator based on current cruise control state
    IndicatorLed::setState(currentState);

    bool buttonEvent = hasButtonEvent(button);
    if(buttonEvent) {
        Debug::printf("Button pressed: %d, state: %d\n", button, currentState);
    }

    float throttlePedal = Throttle::readPedalValue();

    switch(currentState) {
        case SystemState::STATE_OFF:
            // Handle requests to set or resume cruise from OFF state
            if(buttonEvent && (button == CCButton::BUTTON_SET || button == CCButton::BUTTON_RESUME)) {
                bool isResume = button == CCButton::BUTTON_RESUME;
                if(canEnableCruise(isResume, vehicleState)) {
                    
                    if(!isResume) {
                        targetSpeed = vehicleState.speed;
                    }

                    Throttle::setGeneratedValue(0);
                    cruisePID.reset();

                    if(throttlePedal > THROTTLE_PEDAL_THRESHOLD_ENABLE) {
                        Throttle::enableOverride(false);
                        currentState = SystemState::STATE_OVERRIDE;
                    } else {
                        Throttle::enableOverride(true);
                        currentState = SystemState::STATE_ACTIVE;
                    }
                }
            }
            break;

        case SystemState::STATE_OVERRIDE:
            // When the driver presses the pedal, disable cruise and follow the pedal
            if(!canEnableCruise(false, vehicleState) || button == CCButton::BUTTON_CANCEL) {
                Throttle::setGeneratedValue(0);
                Throttle::enableOverride(false);
                cruisePID.reset();
                currentState = SystemState::STATE_OFF;
            } else {
                if(throttlePedal < THROTTLE_PEDAL_THRESHOLD_DISABLE) {
                    Throttle::setGeneratedValue(0);
                    cruisePID.reset();
                    Throttle::enableOverride(true);
                    currentState = SystemState::STATE_ACTIVE;
                }
            }
            break;

        case SystemState::STATE_ACTIVE:
            // Maintain target speed with PID while cruise is active
            if(!canEnableCruise(false, vehicleState) || button == CCButton::BUTTON_CANCEL) {
                Throttle::setGeneratedValue(0);
                Throttle::enableOverride(false);
                cruisePID.reset();
                currentState = SystemState::STATE_OFF;
            } else {
                if(throttlePedal > THROTTLE_PEDAL_THRESHOLD_ENABLE) {
                    Throttle::setGeneratedValue(0);
                    cruisePID.reset();
                    Throttle::enableOverride(false);
                    currentState = SystemState::STATE_OVERRIDE;
                } else {
                    if(buttonEvent) {
                        if(button == CCButton::BUTTON_SET) {
                            targetSpeed--;
                        } else if(button == CCButton::BUTTON_RESUME) {
                            targetSpeed++;
                        }
                        targetSpeed = constrain(targetSpeed, MIN_SPEED_KMH, MAX_SPEED_KMH);
                    }

                    float pidValue = cruisePID.compute(targetSpeed, vehicleState.speed);
                    float throttleValue = pidValue * 100.0f;
                    Throttle::setGeneratedValue(throttleValue);
                }
            }
            break;
    }
}
