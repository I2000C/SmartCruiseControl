#pragma once
#include "can_data.h"
#include "can_decoder.h"

#define REFRESH_COMPUTED_DATA_TIME_MS 100

class CanReader {
    public:
        bool init();

        void readState(VehicleState& state);
    
    private:
        SharedVehicleState sharedState;
        QueueHandle_t canQueue;
        CanDecoder canDecoder;

        friend void canProcessingTask(void* args);
        friend void canRxTask(void* args);
};
