#pragma once
#include <stdint.h>
#include "can_data.h"

struct PidResponse {
    uint8_t data[8];
    uint8_t len;
};

bool processPid(uint8_t pid, PidResponse& response, const VehicleState& state);

bool processExtraPid(uint8_t pid, PidResponse& response, const VehicleState& state);

bool isPidSupported(uint8_t pid);

void buildPidBitmap(uint8_t startPid, uint8_t bitmap[4]);
