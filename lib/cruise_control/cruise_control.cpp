#include "cruise_control.h"
#include "indicator_led.h"
#include "throttle.h"
#include "brake_clutch.h"
#include "debug.h"

void showDisableCauseOnIndicatorLed(uint8_t disableCause) {
    if(!Debug::isEnabled()) {
        return;
    }

    if(disableCause > 12) {
        return;
    }

    IndicatorLed::setState(SystemState::STATE_OFF);
    delay(LED_INITIAL_WAIT_TIME);

    for(int i=3; i>=0; i--) {
        bool bit = (disableCause >> i) & 1;
        IndicatorLed::setState(SystemState::STATE_ACTIVE);
        delay(bit ? LED_LONG_PULSE : LED_SHORT_PULSE);
        IndicatorLed::setState(SystemState::STATE_OFF);

        if(i > 0) {
            delay(LED_WAIT_TIME);
        }
    }
}

uint8_t CruiseControl::canEnableCruise(bool isResume, const VehicleState& vehicleState) {
    uint32_t time = millis();

    // Check software brake and clutch
    if(BrakeClutch::isBrakePressed()) {
        Debug::printf("[%7d] Cruise control was disabled due to BRAKE pedal (SOFTWARE)\n", time);
        return BRAKE_PRESSED_SOFTWARE;
    }
    if(BrakeClutch::isClutchPressed()) {
        Debug::printf("[%7d] Cruise control was disabled due to CLUTCH pedal (SOFTWARE)\n", time);
        return CLUTCH_PRESSED_SOFTWARE;
    }

    // Check hardware brake and clutch
    if(Throttle::isOverrideEnabled() && !Throttle::isMosfetRelayActive()) {
        Debug::printf("[%7d] Cruise control was disabled due to BRAKE or CLUTCH pedals (HARDWARE)\n", time);
        return BRAKE_OR_CLUTCH_PRESSED_HARDWARE;
    }

    // Check brake pedal intensity over CAN bus
    if(vehicleState.brakeIntensity > BRAKE_PEDAL_THRESHOLD) {
        Debug::printf("[%7d] Cruise control was disabled due to BRAKE pedal (CAN_BUS)\n", time);
        return BRAKE_PRESSED_CAN_BUS;
    }

    // Check parking brake and reverse light
    if(vehicleState.parkingBrake) {
        Debug::printf("[%7d] Cruise control was disabled due to PARKING_BRAKE\n", time);
        return PARKING_BRAKE;
    }
    if(vehicleState.reverseLight) {
        Debug::printf("[%7d] Cruise control was disabled due to REVERSE_LIGHT\n", time);
        return REVERSE_LIGHT;
    }

    // Validate vehicle state before enabling or resuming cruise control
    if(!vehicleState.speedValid) {
        Debug::printf("[%7d] Cruise control was disabled due to invalid speed (CAN_BUS)\n", time);
        return SPEED_INVALID_CAN_BUS;
    }

    float currentSpeed = vehicleState.speed;
    if(currentSpeed < MIN_CAR_SPEED_KMH || currentSpeed > MAX_CAR_SPEED_KMH) {
        Debug::printf("[%7d] Cruise control was disabled due to invalid speed (OUT_OF_RANGE)\n", time);
        return SPEED_OUT_OF_RANGE;
    }
    if(isResume) {
        if(targetSpeed < MIN_SPEED_KMH || targetSpeed > MAX_SPEED_KMH) {
            Debug::printf("[%7d] Cruise control was disabled due to invalid RESUME speed (OUT_OF_RANGE)\n", time);
            return RESUME_SPEED_OUT_OF_RANGE;
        }
        if(fabsf(currentSpeed - targetSpeed) > MAX_TARGET_SPEED_DIFF) {
            Debug::printf("[%7d] Cruise control was disabled due to invalid RESUME speed (DIFF_OUT_OF_RANGE)\n", time);
            return RESUME_SPEED_DIFF_OUT_OF_RANGE;
        }
    }

    // Validate RPM freshness and range
    uint32_t rpmLastUpdate = vehicleState.rpmLastUpdate;
    if(millis() - rpmLastUpdate > RPM_TIMEOUT_MS) {
        Debug::printf("[%7d] Cruise control was disabled due to invalid rpm (CAN_BUS)\n", time);
        return RPM_INVALID_CAN_BUS;
    }
    uint16_t rpm = vehicleState.rpm;
    if(rpm < MIN_RPM || rpm > MAX_RPM) {
        Debug::printf("[%7d] Cruise control was disabled due to invalid rpm (OUT_OF_RANGE)\n", time);
        return RPM_INVALID_OUT_OF_RANGE;
    }

    return NO_DISABLE_CAUSE;
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

void CruiseControl::loop(const VehicleState& vehicleState, const Button rawButton) {
    // Update indicator based on current cruise control state
    IndicatorLed::setState(currentState);

    CCButton button = Buttons::parseButton(rawButton);

    bool buttonEvent = hasButtonEvent(button);
    if(buttonEvent) {
        uint32_t time = millis();
        Debug::printf("[%7d] Button pressed: %d, state: %d\n", time, button, currentState);

        // Toggle debug mode if required
        if(rawButton == Button::BUTTON_MUTE) {
            if(Debug::isEnabled()) {
                Debug::println("Debug mode is disabled. ELM327 mode is enabled");
                Debug::setEnabled(false);
            } else {
                Debug::setEnabled(true);
                Debug::println("Debug mode is enabled. ELM327 mode is disabled");
            }
        }
    }

    uint8_t disableCause;
    float throttlePedal = Throttle::readPedalValue();

    switch(currentState) {
        case SystemState::STATE_OFF:
            // Handle requests to set or resume cruise from OFF state
            if(buttonEvent && (button == CCButton::BUTTON_SET || button == CCButton::BUTTON_RESUME)) {
                bool isResume = button == CCButton::BUTTON_RESUME;
                disableCause = canEnableCruise(isResume, vehicleState);
                if(disableCause == NO_DISABLE_CAUSE) {
                    if(!isResume) {
                        targetSpeed = vehicleState.speed;
                    }

                    // Save throttle pedal value to start PID with it
                    throttleStartValue = throttlePedal;

                    Throttle::setGeneratedValue(0);
                    cruisePID.reset();

                    if(throttlePedal > THROTTLE_PEDAL_THRESHOLD_ENABLE) {
                        Throttle::enableOverride(false);
                        currentState = SystemState::STATE_OVERRIDE;
                    } else {
                        Throttle::enableOverride(true);
                        currentState = SystemState::STATE_ACTIVE;
                    }
                } else {
                    showDisableCauseOnIndicatorLed(disableCause);
                }
            }
            break;

        case SystemState::STATE_OVERRIDE:
            // When the driver presses the pedal, disable cruise and follow the pedal
            disableCause = canEnableCruise(false, vehicleState);
            if(disableCause != NO_DISABLE_CAUSE || button == CCButton::BUTTON_CANCEL) {
                Throttle::setGeneratedValue(0);
                Throttle::enableOverride(false);
                cruisePID.reset();
                currentState = SystemState::STATE_OFF;
                showDisableCauseOnIndicatorLed(disableCause);
            } else {
                if(throttlePedal < THROTTLE_PEDAL_THRESHOLD_DISABLE) {
                    Throttle::setGeneratedValue(throttleStartValue);
                    cruisePID.reset();
                    cruisePID.setInitialOutput(throttleStartValue / 100.0f, targetSpeed, vehicleState.speed);
                    Throttle::enableOverride(true);
                    currentState = SystemState::STATE_ACTIVE;
                } else {
                    if(buttonEvent) {
                        // If driver is pressing throttle pedal and SET button is pressed, set target speed to current speed
                        if(button == CCButton::BUTTON_SET) {
                            // Save throttle pedal value to start PID with it
                            throttleStartValue = throttlePedal;
                            targetSpeed = vehicleState.speed;
                            targetSpeed = constrain(targetSpeed, MIN_SPEED_KMH, MAX_SPEED_KMH);
                        }
                    }
                }
            }
            break;

        case SystemState::STATE_ACTIVE:
            // Maintain target speed with PID while cruise is active
            disableCause = canEnableCruise(false, vehicleState);
            if(disableCause != NO_DISABLE_CAUSE || button == CCButton::BUTTON_CANCEL) {
                Throttle::setGeneratedValue(0);
                Throttle::enableOverride(false);
                cruisePID.reset();
                currentState = SystemState::STATE_OFF;
                showDisableCauseOnIndicatorLed(disableCause);
            } else {
                if(throttlePedal > THROTTLE_PEDAL_THRESHOLD_ENABLE) {
                    Throttle::setGeneratedValue(0);
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
