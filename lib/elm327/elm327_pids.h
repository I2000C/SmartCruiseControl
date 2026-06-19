#pragma once
#include <stdint.h>
#include "can_data.h"

struct PidResponse {
    uint8_t data[8];
    uint8_t len;
};

// Process a standard OBD-II PID and produce a response payload
bool processPid(uint8_t pid, PidResponse& response, const VehicleState& state);

// Process an extended OBD-II PID and produce a response payload
bool processExtraPid(uint8_t pid, PidResponse& response, const VehicleState& state);

// Return true if the PID is supported by this emulator
bool isPidSupported(uint8_t pid);

// Build a 32-bit bitmap of supported PIDs following the requested PID
void buildPidBitmap(uint8_t startPid, uint8_t bitmap[4]);
