#include "can_decoder.h"

bool CanDecoder::decodeFrame(const CanFrame& frame, SharedVehicleState& sharedState) {
    switch(frame.id) {
        case 0x09E:
            // Total distance
            ENTER_CRITICAL(sharedState);
            sharedState.state.totalDistance = ((frame.data[4] << 16) | (frame.data[5] << 8) | (frame.data[6])) / 10.0f;

            if(sharedState.state.startDistance < 0) {
                sharedState.state.startDistance = sharedState.state.totalDistance;
            }

            sharedState.state.tripDistance = sharedState.state.totalDistance - sharedState.state.startDistance;
            EXIT_CRITICAL(sharedState);
            break;
        case 0x200:
            // Speed A, B and C
            {
                uint16_t speedA = (frame.data[2] << 8) | (frame.data[3]);
                uint16_t speedB = (frame.data[4] << 8) | (frame.data[5]);
                uint16_t speedC = (frame.data[6] << 8) | (frame.data[7]);
                speedEstimator.updateFromFrame200(speedA, speedB, speedC);
                speedEstimator.compute();
            }

            ENTER_CRITICAL(sharedState);
            sharedState.state.speed = speedEstimator.getSpeed();
            sharedState.state.speedValid = speedEstimator.isValid();
            EXIT_CRITICAL(sharedState);
            break;
        case 0x208:
            // Brake pedal intensity + speed D and E
            {
                uint16_t speedD = (frame.data[4] << 8) | (frame.data[5]);
                uint16_t speedE = (frame.data[6] << 8) | (frame.data[7]);
                speedEstimator.updateFromFrame208(speedD, speedE);
                speedEstimator.compute();
            }

            ENTER_CRITICAL(sharedState);
            sharedState.state.brakeIntensity = ((frame.data[2] << 8) | (frame.data[3]));
            sharedState.state.brakeIntensity = (sharedState.state.brakeIntensity - MIN_BRAKE_VALUE) / (MAX_BRAKE_VALUE - MIN_BRAKE_VALUE) * 100.0f;
            sharedState.state.brakeIntensity = constrain(sharedState.state.brakeIntensity, 0.0f, 100.0f);

            sharedState.state.speed = speedEstimator.getSpeed();
            sharedState.state.speedValid = speedEstimator.isValid();
            EXIT_CRITICAL(sharedState);
            break;
        case 0x210:
            // Throttle pedal intensity
            ENTER_CRITICAL(sharedState);
            sharedState.state.throttleIntensity = frame.data[2] * 100.0f / MAX_THROTTLE_VALUE;
            sharedState.state.throttleIntensity = constrain(sharedState.state.throttleIntensity, 0.0f, 100.0f);
            EXIT_CRITICAL(sharedState);
            break;
        case 0x308:
            // Engine rpm
            ENTER_CRITICAL(sharedState);
            sharedState.state.rpm = (frame.data[1] << 8) | (frame.data[2]);
            EXIT_CRITICAL(sharedState);
            break;
        case 0x408:
            // Fuel level
            ENTER_CRITICAL(sharedState);
            sharedState.state.fuelLevel = frame.data[0];
            EXIT_CRITICAL(sharedState);
            break;
        case 0x416:
            // Battery voltage
            ENTER_CRITICAL(sharedState);
            sharedState.state.batteryVoltage = frame.data[0];
            EXIT_CRITICAL(sharedState);
            break;
        case 0x423:
            // Door status
            ENTER_CRITICAL(sharedState);
            sharedState.state.doorStatus = frame.data[2];
            EXIT_CRITICAL(sharedState);
            break;
        case 0x608:
            // Refrigerant temperature and fuel rate
            ENTER_CRITICAL(sharedState);
            sharedState.state.refrigerantTemperature = frame.data[0] - 40;
            sharedState.state.fuelRate = ((frame.data[5] << 8) | (frame.data[6])) / 100.0f;
            EXIT_CRITICAL(sharedState);

            if(sharedState.state.speedValid) {
                fuelRangeEstimator.updateConsumption(sharedState.state.fuelRate, sharedState.state.speed);
            }
            break;
        default:
            return false;
    }

    return true;
}

void CanDecoder::refreshComputedData(SharedVehicleState& sharedState) {
    // Refresh speed estimator
    speedEstimator.compute();
    ENTER_CRITICAL(sharedState);
    sharedState.state.speed = speedEstimator.getSpeed();
    sharedState.state.speedValid = speedEstimator.isValid();
    EXIT_CRITICAL(sharedState);

    // Refresh trip time
    ENTER_CRITICAL(sharedState);
    if(sharedState.state.startTime == 0) {
        sharedState.state.startTime = millis();
    }
    sharedState.state.tripTime = millis() - sharedState.state.startTime;
    EXIT_CRITICAL(sharedState);

    // Refresh average speed
    float tripDistanceKM = sharedState.state.tripDistance;
    float tripTimeHours = (sharedState.state.tripTime / 1000.0f) / 3600.0f;
    ENTER_CRITICAL(sharedState);
    sharedState.state.averageSpeed = tripTimeHours > 0.0f ? (tripDistanceKM / tripTimeHours) : 0.0f;
    EXIT_CRITICAL(sharedState);

    // Refresh instant fuel rate (L/100km), average fuel rate (L/100km) and range (km)
    float litresRemaing = sharedState.state.fuelLevel;
    ENTER_CRITICAL(sharedState);
    sharedState.state.instantFuelConsumption = fuelRangeEstimator.getInstantConsumption();
    sharedState.state.averageFuelConsumption = fuelRangeEstimator.getAverageConsumption();
    sharedState.state.remainingRange = fuelRangeEstimator.estimateRange(litresRemaing);
    EXIT_CRITICAL(sharedState);
}
