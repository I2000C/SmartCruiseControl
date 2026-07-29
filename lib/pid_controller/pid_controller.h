#pragma once
#include "constants.h"

// Simple PID controller with output saturation and rate limiting
class PIDController {
    private:
        float _kp;
        float _ki;
        float _kd;

        float _dt;
        
        float _maxDeltaUp;
        float _maxDeltaDown;

        float _integral = 0.0f;
        float _prevDeriv = 0.0f;
        float _prevProcessValue = 0.0f;
        float _prevOutput = 0.0f;
        float _prevError = 0.0f;

        float _integralMax;

        float _outMin = 0.0f;
        float _outMax = 1.0f;

        bool _firstRun = true;
    public:
        PIDController(float kp,
                      float ki,
                      float kd,
                      float dt,
                      float maxDeltaUp,
                      float maxDeltaDown)
            : _kp(kp),
              _ki(ki),
              _kd(kd),
              _dt(dt),
              _maxDeltaUp(maxDeltaUp),
              _maxDeltaDown(maxDeltaDown),
              _integralMax(_ki > 0.0f ? (MAX_INTEGRAL_CONTRIBUTION / _ki) : 0.0f) { }

        // Update PID gains
        void setTunings(float kp, float ki, float kd);

        // Reset controller state
        void reset();

        // Compute PID output for given setpoint and process value.
        // Output value is between _outMin and _outMax
        float compute(float setpoint, float processValue);
};
