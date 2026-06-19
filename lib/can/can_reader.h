#pragma once
#include "can_data.h"
#include "can_decoder.h"

#define REFRESH_COMPUTED_DATA_TIME_MS 100 // Interval to refresh computed vehicle data

// CAN reader handles raw CAN reception and exposes decoded vehicle state
class CanReader {
    public:
        // Initialize TWAI driver and start CAN tasks
        bool init();

        // Thread-safe copy of the latest vehicle state
        void readState(VehicleState& state) const;
    
    private:
        SharedVehicleState sharedState;   // Shared state protected by seq lock
        QueueHandle_t canQueue;           // Queue for raw CAN frames
        CanDecoder canDecoder;            // Decoder for CAN payloads

        friend void canProcessingTask(void* args); // Task may access private members
        friend void canRxTask(void* args);
};
