#pragma once
#include "constants.h"
#include "state.h"
#include "buttons.h"
#include "pid_controller.h"
#include "can_data.h"

// Constant values to show disable cause on indicator led if debug mode is enabled
#define NO_DISABLE_CAUSE 0
#define BRAKE_PRESSED_SOFTWARE 1
#define CLUTCH_PRESSED_SOFTWARE 2
#define BRAKE_OR_CLUTCH_PRESSED_HARDWARE 3
#define BRAKE_PRESSED_CAN_BUS 4
#define PARKING_BRAKE 5
#define REVERSE_LIGHT 6
#define SPEED_INVALID_CAN_BUS 7
#define SPEED_OUT_OF_RANGE 8
#define RESUME_SPEED_OUT_OF_RANGE 9
#define RESUME_SPEED_DIFF_OUT_OF_RANGE 10
#define RPM_INVALID_CAN_BUS 11
#define RPM_INVALID_OUT_OF_RANGE 12

#define LED_SHORT_PULSE 200
#define LED_LONG_PULSE 600
#define LED_INITIAL_WAIT_TIME 1000
#define LED_WAIT_TIME 300

// Cruise control state machine and control loop
class CruiseControl {
    private:
        SystemState currentState = SystemState::STATE_OFF;
        CCButton previousButton = CCButton::BUTTON_NONE;

        uint16_t buttonHoldTicks = 0;

        float throttleStartValue = 0.0f;

        float targetSpeed = 0.0f;
        PIDController cruisePID;

        // Validate whether cruise can be enabled or resumed
        uint8_t canEnableCruise(bool isResume, const VehicleState& vehicleState);
        bool hasButtonEvent(CCButton button);

    public:
        // Run one iteration of cruise control logic
        void loop(const VehicleState& vehicleState, const Button rawButton);

        CruiseControl() : cruisePID(PID_KP, PID_KI, PID_KD,
                                    MAIN_LOOP_PERIOD_MS / 1000.0f,
                                    THROTTLE_MAX_DELTA_UP,
                                    THROTTLE_MAX_DELTA_DOWN) { }
};
