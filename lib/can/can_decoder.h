#pragma once
#include <Arduino.h>
#include "can_data.h"
#include "speed_estimator.h"
#include "fuel_range_estimator.h"

// CAN decoder transforms raw CAN frames into vehicle state values
class CanDecoder {
    public:
        // Decode one CAN frame and update shared state
        bool decodeFrame(const CanFrame& frame, SharedVehicleState& state);

        // Refresh derived state values that require periodic recomputation
        void refreshComputedData(SharedVehicleState& state);
    private:
        SpeedEstimator speedEstimator;        // Speed fusion and timeout logic
        FuelRangeEstimator fuelRangeEstimator; // Fuel consumption estimation

        // Sequence lock helpers for writing shared state safely
        inline void ENTER_CRITICAL(SharedVehicleState& sharedState) {
            sharedState.seq.fetch_add(1, std::memory_order_relaxed);
        }
        
        inline void EXIT_CRITICAL(SharedVehicleState& sharedState) {
            sharedState.seq.fetch_add(1, std::memory_order_release);
        }
};
