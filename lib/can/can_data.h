#pragma once
#include <stdint.h>
#include <atomic>

#define MAX_THROTTLE_VALUE 0xA1

#define MIN_BRAKE_VALUE 0x4000
#define MAX_BRAKE_VALUE 0x4659

struct CanFrame {
    uint32_t id;
    uint8_t data[8];
    uint8_t dlc;
    uint32_t timestamp;
};

struct VehicleState {
    // Critical data
    float speed;        // km/h
    bool speedValid;
    uint16_t rpm;       // rpm
    uint32_t rpmLastUpdate;

    float totalDistance;    // km
    float startDistance;    // km
    float tripDistance;     // km
    uint32_t startTime;     // millis
    uint32_t tripTime;      // millis

    float brakeIntensity;       // %
    float throttleIntensity;    // %

    uint8_t fuelLevel;          // L
    float batteryVoltage;       // V

    // Door status
    // bit 0 --> Right front door
    // bit 1 --> Left front door
    // bit 3 --> Right rear door
    // bit 4 --> Left rear door
    // bit 6 --> Trunk door
    uint8_t doorStatus;

    int16_t refrigerantTemperature;     // ºC
    float fuelRate;                     // L/h

    float averageSpeed;             // km/h
    float instantFuelConsumption;   // L/100 km
    float averageFuelConsumption;   // L/100 km
    float remainingRange;           // km
};

struct SharedVehicleState {
    std::atomic<uint32_t> seq{0};
    VehicleState state;
};
