#include "elm327_pids.h"
#include "constants.h"
#include <cinttypes>
#include <cmath>

static bool pidCoolant(PidResponse& out, const VehicleState& state) {
    int16_t temperature = state.refrigerantTemperature;
    uint8_t value = (uint8_t) (temperature + 40);
    out.data[0] = value;
    out.len = 1;
    return true;
}

static bool pidRPM(PidResponse& out, const VehicleState& state) {
    uint16_t rpm = state.rpm;
    if(millis() - state.rpmLastUpdate > RPM_TIMEOUT_MS) {
        rpm = 0;
    }
    out.data[0] = (rpm >> 8) & 0xFF;
    out.data[1] = rpm & 0xFF;
    out.len = 2;
    return true;
}

static bool pidSpeed(PidResponse& out, const VehicleState& state) {
    float speed = state.speed;
    if(state.speedValid) {
        speed = 0.0f;
    }
    out.data[0] = (uint8_t) round(speed);
    out.len = 1;
    return true;
}

static bool pidThrottle(PidResponse& out, const VehicleState& state) {
    float throttle = state.throttleIntensity;
    out.data[0] = (uint8_t) fmin(round(throttle * 255 / 100), 255.0f);
    out.len = 1;
    return true;
}

static bool pidVoltage(PidResponse& out, const VehicleState& state) {
    float voltage = state.batteryVoltage;
    uint16_t value = (uint16_t) round(voltage * 10);
    out.data[0] = (value >> 8) & 0xFF;
    out.data[1] = value & 0xFF;
    out.len = 2;
    return true;
}

static bool pidFuelRate(PidResponse& out, const VehicleState& state) {
    float fuelRate = state.fuelRate;
    uint16_t value = (uint16_t) round(fuelRate * 20);
    out.data[0] = (value >> 8) & 0xFF;
    out.data[1] = value & 0xFF;
    out.len = 2;
    return true;
}

// ------------------------------------------------------------

static bool extraPidTotalDistance(PidResponse& out, const VehicleState& state) {
    float totalDistance = state.totalDistance;
    uint32_t value = (uint32_t) round(totalDistance * 10);
    out.data[0] = (value >> 16) & 0xFF;
    out.data[1] = (value >> 8) & 0xFF;
    out.data[2] = value & 0xFF;
    out.len = 3;
    return true;
}

static bool extraPidTripDistance(PidResponse& out, const VehicleState& state) {
    float tripDistance = state.tripDistance;
    uint32_t value = (uint32_t) round(tripDistance * 10);
    out.data[0] = (value >> 8) & 0xFF;
    out.data[1] = value & 0xFF;
    out.len = 2;
    return true;
}

static bool extraPidTripTimeHours(PidResponse& out, const VehicleState& state) {
    uint32_t tripTimeMillis = state.tripTime;
    uint32_t tripTimeSeconds = tripTimeMillis / 1000;
    uint8_t value = (uint8_t) tripTimeSeconds / 3600;
    out.data[0] = value;
    out.len = 1;
    return true;
}

static bool extraPidTripTimeMinutes(PidResponse& out, const VehicleState& state) {
    uint32_t tripTimeMillis = state.tripTime;
    uint32_t tripTimeSeconds = tripTimeMillis / 1000;
    uint8_t value = (uint8_t) ((tripTimeSeconds % 3600) / 60);
    out.data[0] = value;
    out.len = 1;
    return true;
}

static bool extraPidBrake(PidResponse& out, const VehicleState& state) {
    float brake = state.brakeIntensity;
    out.data[0] = (uint8_t) fmin(round(brake * 255 / 100), 255.0f);
    out.len = 1;
    return true;
}

static bool extraPidFuelLevel(PidResponse& out, const VehicleState& state) {
    uint8_t fuelLevel = state.fuelLevel;
    out.data[0] = fuelLevel;
    out.len = 1;
    return true;
}

static bool extraPidDoorStatus(PidResponse& out, const VehicleState& state) {
    uint8_t doorStatus = state.doorStatus;
    out.data[0] = doorStatus;
    out.len = 1;
    return true;
}

static bool extraPidAverageSpeed(PidResponse& out, const VehicleState& state) {
    float averageSpeed = state.averageSpeed;
    out.data[0] = (uint8_t) round(averageSpeed);
    out.len = 1;
    return true;
}

static bool extraPidInstantFuelConsumption(PidResponse& out, const VehicleState& state) {
    float instantFuelConsumption = state.instantFuelConsumption;
    uint16_t value = (uint16_t) round(instantFuelConsumption * 20);
    out.data[0] = (value >> 8) & 0xFF;
    out.data[1] = value & 0xFF;
    out.len = 2;
    return true;
}

static bool extraPidAverageFuelConsumption(PidResponse& out, const VehicleState& state) {
    float averageFuelConsumption = state.averageFuelConsumption;
    uint16_t value = (uint16_t) round(averageFuelConsumption * 20);
    out.data[0] = (value >> 8) & 0xFF;
    out.data[1] = value & 0xFF;
    out.len = 2;
    return true;
}

static bool extraPidRemainingRange(PidResponse& out, const VehicleState& state) {
    float remainingRange = state.remainingRange;
    uint16_t value = (uint16_t) round(remainingRange);
    out.data[0] = (value >> 8) & 0xFF;
    out.data[1] = value & 0xFF;
    out.len = 2;
    return true;
}

// ------------------------------------------------------------

struct PidEntry {
    uint8_t pid;
    bool (*handler)(PidResponse&, const VehicleState& state);
};

// https://es.wikipedia.org/wiki/OBD-II_PID
// https://www.csselectronics.com/pages/obd2-pid-table-on-board-diagnostics-j1979
static const PidEntry table[] =
{
    {0x05, pidCoolant},
    {0x0C, pidRPM},
    {0x0D, pidSpeed},
    {0x11, pidThrottle},
    {0x42, pidVoltage},
    {0x5E, pidFuelRate}
};

static const PidEntry extendedTable[] =
{
    {0x00, extraPidTotalDistance},
    {0x01, extraPidTripDistance},
    {0x02, extraPidTripTimeHours},
    {0x03, extraPidTripTimeMinutes},
    {0x04, extraPidBrake},
    {0x05, extraPidFuelLevel},
    {0x06, extraPidDoorStatus},
    {0x07, extraPidAverageSpeed},
    {0x08, extraPidInstantFuelConsumption},
    {0x09, extraPidAverageFuelConsumption},
    {0x0A, extraPidRemainingRange}
};

static const int tableSize = sizeof(table) / sizeof(table[0]);
static const int extendedTableSize = sizeof(extendedTable) / sizeof(extendedTable[0]);

bool isPidSupported(uint8_t pid) {
    for(int i = 0; i < tableSize; i++) {
        if(table[i].pid == pid) {
            return true;
        }
    }
    return false;
}

bool processPid(uint8_t pid, PidResponse& out, const VehicleState& state) {
    for(int i = 0; i < tableSize; i++) {
        if(table[i].pid == pid) {
            return table[i].handler(out, state);
        }
    }
    return false;
}

bool processExtraPid(uint8_t pid, PidResponse& out, const VehicleState & state) {
    for(int i = 0; i < extendedTableSize; i++) {
        if(extendedTable[i].pid == pid) {
            return extendedTable[i].handler(out, state);
        }
    }
    return false;
}

void buildPidBitmap(uint8_t startPid, uint8_t out[4]) {
    uint32_t bitmap = 0;

    for(uint8_t pid = startPid + 1; pid <= startPid + 32; pid++) {
        if(isPidSupported(pid)) {
            uint8_t offset = pid - startPid - 1;
            bitmap |= (1UL << (31 - offset));
        }
    }

    out[0] = (bitmap >> 24) & 0xFF;
    out[1] = (bitmap >> 16) & 0xFF;
    out[2] = (bitmap >> 8) & 0xFF;
    out[3] = bitmap & 0xFF;
}
