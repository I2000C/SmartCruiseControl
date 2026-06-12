#pragma once
#include <Arduino.h>
#include "can_data.h"
#include "speed_estimator.h"
#include "fuel_range_estimator.h"

class CanDecoder {
    public:
        bool decodeFrame(const CanFrame& frame, SharedVehicleState& state);

        void refreshComputedData(SharedVehicleState& state);
    private:
        SpeedEstimator speedEstimator;
        FuelRangeEstimator fuelRangeEstimator;

        inline void ENTER_CRITICAL(SharedVehicleState& sharedState) {
            sharedState.seq.fetch_add(1, std::memory_order_release);
        }
        
        inline void EXIT_CRITICAL(SharedVehicleState& sharedState) {
            sharedState.seq.fetch_add(1, std::memory_order_release);
        }
};
