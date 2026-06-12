#pragma once

#define MIN_FUEL_LEVEL 5.0f

class FuelRangeEstimator {
    private:
        float emaConsumption;   // L/100 km (exponential moving average)
        float alpha;            // Smoothing factor (0-1)

        float lastInstantConsumption;   // Last L/100 km instant value
    
    public:
        FuelRangeEstimator(float alpha = 0.2f) : alpha(alpha) { }

        // Update using fuel rate (L/h) and vehicle speed (km/h)
        void updateConsumption(float fuelRate_Lh, float speed_kmh) {
            if(speed_kmh <= 5.0f) {
                lastInstantConsumption = 0.0f;
                return; // Avoid division by zero or invalid data at standstill
            }

            // Instant consumption in L/100km
            // (L/h) / (km/h) * 100 = L/100km
            lastInstantConsumption = (fuelRate_Lh / speed_kmh) * 100.0f;

            // Exponential moving average (EMA) filtering
            emaConsumption = alpha * lastInstantConsumption + (1.0f - alpha) * emaConsumption;
        }

        // Smoothed average consumption (L/100 km)
        float getAverageConsumption() const {
            return emaConsumption;
        }

        // Instant consumption (L/100 km) from last sample
        float getInstantConsumption() const {
            return lastInstantConsumption;
        }

        // Estimated driving range (km)
        float estimateRange(float fuelLitersRemaining) const {
            if(emaConsumption <= 0.0f) {
                return 0.0f;
            }
            
            float usableFuel = fuelLitersRemaining - MIN_FUEL_LEVEL;
            
            if(usableFuel <= 0.0f) {
                return 0.0f;
            }
            
            return (usableFuel / emaConsumption) * 100.0f;
        }
};
