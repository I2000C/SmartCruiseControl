#pragma once
#include "constants.h"
#include "state.h"
#include "buttons.h"
#include "pid_controller.h"
#include "can_data.h"

class CruiseControl {
    private:
        SystemState currentState = SystemState::STATE_OFF;
        CCButton lastPressedButton = CCButton::BUTTON_NONE;
        float targetSpeed = 0.0f;
        PIDController cruisePID;

        bool canEnableCruise(bool isResume, const VehicleState& vehicleState);

    public:
        void loop(const VehicleState& vehicleState);

        CruiseControl() : cruisePID(PID_KP, PID_KI, PID_KD,
                                    MAIN_LOOP_PERIOD_MS / 1000.0f, 0.0f, 100.0f,
                                    THROTTLE_MAX_DELTA_UP,
                                    THROTTLE_MAX_DELTA_DOWN) {
                                        cruisePID.setIntegralLimit(500.0f);
                                    }
};
