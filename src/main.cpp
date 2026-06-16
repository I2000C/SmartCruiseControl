#include <Arduino.h>
#include "can_reader.h"
#include "elm327.h"
#include "indicator_led.h"
#include "throttle.h"
#include "brake_clutch.h"
#include "buttons.h"
#include "pid_controller.h"

VehicleState vehicleState;
SystemState currentState = SystemState::STATE_OFF;
float targetSpeed = 0.0f;
CCButton lastPressedButton = CCButton::BUTTONCC_NONE;

CanReader canReader;
Elm327 elm327(canReader);
PIDController cruisePID(PID_KP, PID_KI, PID_KD, 
                        MAIN_LOOP_PERIOD_MS / 1000.0f, 0.0f, 100.0f, 
                        THROTTLE_MAX_DELTA_UP, THROTTLE_MAX_DELTA_DOWN);

bool canEnableCruise(bool isResume) {
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

void mainTask(void* args) {
    const TickType_t period = pdMS_TO_TICKS(MAIN_LOOP_PERIOD_MS);
    TickType_t lastWakeTime = xTaskGetTickCount();

    while(true) {
        IndicatorLed::setState(currentState);
        canReader.readState(vehicleState);
        CCButton button = Buttons::getPressedCCButton();
        bool isNewPulsation = button != lastPressedButton;
        if(isNewPulsation) {
            lastPressedButton = button;
        }
        float throttlePedal = Throttle::readPedalValue();

        switch(currentState) {
            case SystemState::STATE_OFF:
                if(isNewPulsation && (button == CCButton::BUTTONCC_SET || button == CCButton::BUTTONCC_RESUME)) {
                    bool isResume = button == CCButton::BUTTONCC_RESUME;
                    if(canEnableCruise(isResume)) {
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
                if(!canEnableCruise(false) || button == CCButton::BUTTONCC_CANCEL) {
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
                if(!canEnableCruise(false) || button == CCButton::BUTTONCC_CANCEL) {
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
                            if(button == CCButton::BUTTONCC_SET) {
                                targetSpeed--;
                            } else if(button == CCButton::BUTTONCC_RESUME) {
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

        vTaskDelayUntil(&lastWakeTime, period);
    }
}

void setup() {
    Throttle::init();
    BrakeClutch::init();
    IndicatorLed::init();
    canReader.init();
    elm327.init();
    cruisePID.setIntegralLimit(500.0f);

    xTaskCreatePinnedToCore(mainTask, "MainTask", 10000, nullptr, MAIN_TASK_PRIORITY, nullptr, APP_CORE_ID);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
