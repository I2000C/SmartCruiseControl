#pragma once
#include <cstdint>

#define SPEED_TIMEOUT_MS 3000

class SpeedEstimator {
    private:
        struct Source {
            float value = 0.0f;
            uint32_t lastUpdate = 0;
        };

        Source s200;
        Source s208;

        float fusedSpeed = 0.0f;
        bool valid = false;

        bool isValid(uint32_t time) const {
            return (millis() - time) < SPEED_TIMEOUT_MS;
        }

        float weight(uint32_t time) const {
            float dt = (millis() - time) / 1000.0f;
            return expf(-dt);
        }
        
    public:
        void updateFromFrame200(uint16_t speedA, uint16_t speedB, uint16_t speedC) {
            float speed = ((speedA + speedB + speedC) / 3.0f) * 0.05f;
            s200.value = speed;
            s200.lastUpdate = millis();
        }

        void updateFromFrame208(uint16_t speedD, uint16_t speedE) {
            float speed = ((speedD + speedE) / 2.0f) * 0.05f;
            s208.value = speed;
            s208.lastUpdate = millis();
        }

        float getSpeed() const {
            return fusedSpeed;
        }

        bool isValid() const {
            return valid;
        }

        void compute() {
            bool v200 = isValid(s200.lastUpdate);
            bool v208 = isValid(s208.lastUpdate);

            float sum = 0.0f;
            float wsum = 0.0f;

            if(v200) {
                float w = weight(s200.lastUpdate);
                sum += w * s200.value;
                wsum += w;
            }

            if(v208) {
                float w = weight(s208.lastUpdate);
                sum += w * s208.value;
                wsum += w;
            }

            if(wsum > 0.0f) {
                fusedSpeed = sum / wsum;
                valid = true;
            } else {
                valid = false;
            }
        }
};
