#pragma once
#include "can_reader.h"

struct ElmContext {
    bool echo      = false;     // ATE0 / ATE1
    bool headers   = false;     // ATH0 / ATH1
    bool spaces    = true;      // ATS0 / ATS1
    bool linefeeds = true;      // ATL0 / ATL1
};

class Elm327 {
    public:
        Elm327(const CanReader& canReader) : canReader(canReader) { };

        void init();

        void processCommand(const char* command, const VehicleState& vehicleState);

    private:
        const CanReader& canReader;

        ElmContext context;

        void processAT(const char* cmd, const VehicleState& vehicleState);

        void processMode01(const char* cmd, const VehicleState& vehicleState);

        void processMode22(const char* cmd, const VehicleState& vehicleState);

        void sendPrompt();

        void sendText(const char* text);

        void sendResponse(const uint8_t* data, uint8_t len, const char* header = "7E8");

        friend void elm327Task(void* args);
};
