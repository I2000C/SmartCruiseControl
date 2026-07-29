#pragma once
#include "constants.h"
#include "state.h"
#include "buttons.h"
#include "pid_controller.h"
#include "can_data.h"

// Cruise control state machine and control loop
class CruiseControl {
    private:
        SystemState currentState = SystemState::STATE_OFF;
        CCButton lastPressedButton = CCButton::BUTTON_NONE;
        float targetSpeed = 0.0f;
        PIDController cruisePID;

        // Validate whether cruise can be enabled or resumed
        bool canEnableCruise(bool isResume, const VehicleState& vehicleState);

    public:
        // Run one iteration of cruise control logic
        void loop(const VehicleState& vehicleState, const CCButton button);

        CruiseControl() : cruisePID(PID_KP, PID_KI, PID_KD,
                                    MAIN_LOOP_PERIOD_MS / 1000.0f,
                                    THROTTLE_MAX_DELTA_UP,
                                    THROTTLE_MAX_DELTA_DOWN) { }
};
