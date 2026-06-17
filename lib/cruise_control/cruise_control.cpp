#include "cruise_control.h"
#include "brake_clutch.h"
#include "indicator_led.h"
#include "throttle.h"

bool CruiseControl::canEnableCruise(bool isResume, const VehicleState& vehicleState) {
    // Check speed
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

    // Check rpm
    uint16_t rpm = vehicleState.rpm;
    if(rpm < MIN_RPM || rpm > MAX_RPM) {
        return false;
    }

    // Check brake and clutch
    if(BrakeClutch::isPressed()) {
        return false;
    }

    return true;
}

void CruiseControl::loop(const VehicleState& vehicleState) {
    IndicatorLed::setState(currentState);
    CCButton button = Buttons::getPressedCCButton();
    bool isNewPulsation = button != lastPressedButton;
    if(isNewPulsation) {
        lastPressedButton = button;
    }
    float throttlePedal = Throttle::readPedalValue();

    switch(currentState) {
        case SystemState::STATE_OFF:
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

                    float throttle = cruisePID.compute(targetSpeed, vehicleState.speed);
                    Throttle::setGeneratedValue(throttle);
                }
            }
            break;
    }
}
