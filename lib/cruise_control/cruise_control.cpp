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
    if(currentSpeed < MIN_SPEED_KMH || currentSpeed > MAX_SPEED_KMH) {
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

void CruiseControl::loop(const VehicleState& vehicleState, const CCButton button) {
    // Update indicator based on current cruise control state
    IndicatorLed::setState(currentState);

    bool isNewPulsation = button != lastPressedButton;
    if(isNewPulsation) {
        lastPressedButton = button;
    }

    if(button != CCButton::BUTTON_NONE) {
        Debug::printf("Button pressed: %d, new pulsation: %d, state: %d\n", button, isNewPulsation, currentState);
    }

    float throttlePedal = Throttle::readPedalValue();

    switch(currentState) {
        case SystemState::STATE_OFF:
            // Handle requests to set or resume cruise from OFF state
            if(isNewPulsation && (button == CCButton::BUTTON_SET || button == CCButton::BUTTON_RESUME)) {
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
                    if(isNewPulsation) {
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
