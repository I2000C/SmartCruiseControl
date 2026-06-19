#pragma once

// Simple PID controller with output saturation and rate limiting
class PIDController {
    private:
        float _kp;
        float _ki;
        float _kd;

        float _dt;
        
        float _outMin;
        float _outMax;
        float _maxDeltaUp;
        float _maxDeltaDown;

        float _integral = 0.0f;
        float _prevError = 0.0f;
        float _prevDeriv = 0.0f;
        float _prevProcessValue = 0.0f;
        float _prevOutput = 0.0f;

        float _integralMax = 1000.0f;
    public:
        PIDController(float kp,
                      float ki,
                      float kd,
                      float dt,
                      float outMin,
                      float outMax,
                      float maxDeltaUp,
                      float maxDeltaDown)
            : _kp(kp),
              _ki(ki),
              _kd(kd),
              _dt(dt),
              _outMin(outMin),
              _outMax(outMax),
              _maxDeltaUp(maxDeltaUp),
              _maxDeltaDown(maxDeltaDown) { }

        // Update PID gains
        void setTunings(float kp, float ki, float kd);

        // Limit integral windup
        void setIntegralLimit(float limit);

        // Reset controller state
        void reset();

        // Compute PID output for given setpoint and process value
        float compute(float setpoint, float processValue);
};
