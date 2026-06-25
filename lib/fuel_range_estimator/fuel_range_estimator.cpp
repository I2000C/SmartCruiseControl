#include "fuel_range_estimator.h"
#include <array>
#include <cmath>

/*********************/
/* PRIVATE FUNCTIONS */
/*********************/

// Display-friendly rounding
static float roundDisplayedRange(float rangeKm) {
    // Higher values use coarser resolution to avoid
    // unnecessary display fluctuations.
    if(rangeKm < 100.0f) {
        return std::round(rangeKm);
    }

    if(rangeKm < 300.0f) {
        return std::round(rangeKm / 2.0f) * 2.0f;
    }

    return std::round(rangeKm / 5.0f) * 5.0f;
}

void FuelRangeEstimator::addHistoryPoint() {
    history[historyHead] = {totalDistanceKm, totalFuelUsedL};

    historyHead = (historyHead + 1) % HISTORY_SIZE;

    if(historyCount < HISTORY_SIZE) {
        historyCount++;
    }
}

bool FuelRangeEstimator::getWindowConsumption(float windowKm, float& consumption) const {
    if(historyCount < 2) {
        return false;
    }

    const float targetDistance = totalDistanceKm - windowKm;

    const size_t newest = (historyHead + HISTORY_SIZE - 1) % HISTORY_SIZE;

    const HistoryPoint& current = history[newest];

    // Search backwards until a sample older than the
    // requested distance window is found.
    for(size_t i = 0; i < historyCount; ++i) {
        const size_t idx = (historyHead + HISTORY_SIZE - 1 - i) % HISTORY_SIZE;

        const HistoryPoint& sample = history[idx];

        if(sample.totalDistanceKm <= targetDistance) {
            const float distanceDelta = current.totalDistanceKm - sample.totalDistanceKm;
            const float fuelDelta = current.totalFuelUsedL - sample.totalFuelUsedL;

            // Avoid division by very small values.
            if(distanceDelta < 0.1f) {
                return false;
            }

            consumption = (fuelDelta / distanceDelta) * 100.0f;

            return true;
        }
    }

    return false;
}

float FuelRangeEstimator::calculateRangeConsumption() const {
    // Not enough data yet.
    if(totalDistanceKm < 0.1f) {
        return 0.0f;
    }

    // During the first kilometers, use the trip average.
    if(totalDistanceKm < 10.0f) {
        return getTripAverageConsumption();
    }

    float consumption;

    // Gradually transition to moving-window consumption
    // as more trip data becomes available.
    if(totalDistanceKm < 30.0f) {
        if(getWindowConsumption(10.0f, consumption)) {
            return consumption;
        }
    } else {
        if(getWindowConsumption(30.0f, consumption)) {
            return consumption;
        }
    }

    // Fallback if window calculation fails.
    return getTripAverageConsumption();
}

float FuelRangeEstimator::calculateRawRange(float fuelRemainingL) const {
    // Keep a reserve fuel amount that is not considered
    // available for range estimation.
    const float usableFuel = fuelRemainingL - MIN_FUEL_LEVEL;

    if(usableFuel <= 0.0f) {
        return 0.0f;
    }

    const float consumption = calculateRangeConsumption();

    if(consumption < 0.1f) {
        return 0.0f;
    }

    return (usableFuel / consumption) * 100.0f;
}

/********************/
/* PUBLIC FUNCTIONS */
/********************/

void FuelRangeEstimator::update(float fuelRateLh,
                                float speedKmh,
                                float odometerKm,
                                float fuelRemainingL,
                                float dtSeconds) {

    //-----------------------------------------------------
    // Accumulate fuel consumption
    //-----------------------------------------------------

    totalFuelUsedL += fuelRateLh * (dtSeconds / 3600.0f);

    //-----------------------------------------------------
    // Filter instantaneous consumption
    //-----------------------------------------------------

    if(speedKmh > 5.0f) {
        const float instant = (fuelRateLh / speedKmh) * 100.0f;

        if(instantConsumptionFiltered <= 0.0f) {
            instantConsumptionFiltered = instant;
        } else {
            instantConsumptionFiltered = ALPHA_INSTANT * instant +
                (1.0f - ALPHA_INSTANT) * instantConsumptionFiltered;
        }
    }

    //-----------------------------------------------------
    // Track travelled distance
    //-----------------------------------------------------

    if(lastOdometerKm >= 0.0f) {
        const float deltaKm = odometerKm - lastOdometerKm;

        // Ignore unrealistic odometer jumps that may
        // occur due to initialization or sensor issues.
        if(deltaKm > 0.0f && deltaKm < 5.0f) {
            totalDistanceKm += deltaKm;

            addHistoryPoint();
        }
    }

    lastOdometerKm = odometerKm;

    //-----------------------------------------------------
    // Slow update of displayed range
    //-----------------------------------------------------

    rangeUpdateTimerS += dtSeconds;

    if(rangeUpdateTimerS >= RANGE_UPDATE_PERIOD_S) {
        rangeUpdateTimerS = 0.0f;

        const float rawRange = calculateRawRange(fuelRemainingL);

        if(displayedRangeKm <= 0.0f) {
            displayedRangeKm = rawRange;
        } else {
            displayedRangeKm = RANGE_DISPLAY_ALPHA * rawRange +
                (1.0f - RANGE_DISPLAY_ALPHA) * displayedRangeKm;
        }

        displayedRangeKm = roundDisplayedRange(displayedRangeKm);
    }
}

float FuelRangeEstimator::getInstantConsumption() const {
    return instantConsumptionFiltered;
}

float FuelRangeEstimator::getTripAverageConsumption() const {
    if(totalDistanceKm < 0.1f) {
        return 0.0f;
    }

    return (totalFuelUsedL / totalDistanceKm) * 100.0f;
}

float FuelRangeEstimator::getRangeConsumption() const {
    return calculateRangeConsumption();
}

float FuelRangeEstimator::getDisplayedRangeKm() const {
    return displayedRangeKm;
}

float FuelRangeEstimator::getRawRangeKm(float fuelRemainingL) const {
    return calculateRawRange(fuelRemainingL);
}

//---------------------------------------------------------
// Trip statistics
//---------------------------------------------------------

float FuelRangeEstimator::getDistanceTravelledKm() const {
    return totalDistanceKm;
}

float FuelRangeEstimator::getFuelUsedLiters() const {
    return totalFuelUsedL;
}

//---------------------------------------------------------
// Reset trip-related data
//---------------------------------------------------------

void FuelRangeEstimator::resetTrip() {
    totalFuelUsedL = 0.0f;
    totalDistanceKm = 0.0f;

    historyHead = 0;
    historyCount = 0;

    instantConsumptionFiltered = 0.0f;

    displayedRangeKm = 0.0f;
    rangeUpdateTimerS = 0.0f;
}
