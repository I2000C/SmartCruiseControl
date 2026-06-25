#pragma once

#include <array>
#include <cmath>

#define MIN_FUEL_LEVEL 5.0f

class FuelRangeEstimator {
    private:

        struct HistoryPoint {
            float totalDistanceKm;
            float totalFuelUsedL;
        };

        //---------------------------------------------------------
        // Configuration
        //---------------------------------------------------------

        // Smoothing factor for instantaneous consumption filtering.
        static constexpr float ALPHA_INSTANT = 0.02f;

        // Number of samples stored in the circular history buffer.
        static constexpr size_t HISTORY_SIZE = 500;

        // Update displayed range every 5 seconds.
        static constexpr float RANGE_UPDATE_PERIOD_S = 5.0f;

        // Low-pass filter coefficient for displayed range.
        static constexpr float RANGE_DISPLAY_ALPHA = 0.20f;

        //---------------------------------------------------------
        // Instantaneous consumption
        //---------------------------------------------------------

        // Filtered fuel consumption in L/100 km.
        float instantConsumptionFiltered = 0.0f;

        //---------------------------------------------------------
        // Trip accumulators
        //---------------------------------------------------------

        // Total fuel consumed since trip reset.
        float totalFuelUsedL = 0.0f;

        // Total distance travelled since trip reset.
        float totalDistanceKm = 0.0f;

        //---------------------------------------------------------
        // Odometer tracking
        //---------------------------------------------------------

        float lastOdometerKm = -1.0f;

        //---------------------------------------------------------
        // Circular history buffer
        //---------------------------------------------------------

        std::array<HistoryPoint, HISTORY_SIZE> history{};

        size_t historyHead = 0;
        size_t historyCount = 0;

        //---------------------------------------------------------
        // Displayed range
        //---------------------------------------------------------

        // Smoothed range value shown to the driver.
        float displayedRangeKm = 0.0f;

        float rangeUpdateTimerS = 0.0f;

    private:

        // Add a history sample
        void addHistoryPoint();

        // Calculate consumption over a moving distance window
        bool getWindowConsumption(float windowKm, float& consumption) const;

        // Consumption used for range estimation
        float calculateRangeConsumption() const;

        // Raw (unsmoothed) range estimation
        float calculateRawRange(float fuelRemainingL) const;

    public:

        FuelRangeEstimator() = default;

        // Periodic update (typically called every 100 ms)
        void update(float fuelRateLh,
                    float speedKmh,
                    float odometerKm,
                    float fuelRemainingL,
                    float dtSeconds);

        // Instantaneous consumption (L/100 km)
        float getInstantConsumption() const;

        // Trip average consumption (L/100 km)
        float getTripAverageConsumption() const;

        // Consumption currently used for range estimation
        float getRangeConsumption() const;

        // Smoothed range displayed to the driver
        float getDisplayedRangeKm() const;

        // Raw range estimation (for debugging purposes)
        float getRawRangeKm(float fuelRemainingL) const;

        //---------------------------------------------------------
        // Trip statistics
        //---------------------------------------------------------

        float getDistanceTravelledKm() const;
        float getFuelUsedLiters() const;

        //---------------------------------------------------------
        // Reset trip-related data
        //---------------------------------------------------------

        void resetTrip();
};
