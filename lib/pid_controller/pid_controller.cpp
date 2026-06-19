#include "pid_controller.h"
#include <Arduino.h>

void PIDController::setTunings(float kp, float ki, float kd) {
    // Update controller gains
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

void PIDController::setIntegralLimit(float limit) {
    // Set maximum absolute integral term to prevent windup
    _integralMax = limit;
}

void PIDController::reset() {
    // Clear historic PID state
    _integral = 0.0f;
    _prevError = 0.0f;
    _prevDeriv = 0.0f;
    _prevProcessValue = 0.0f;
    _prevOutput = 0.0f;
}

float PIDController::compute(float setpoint, float processValue) {
    float error = setpoint - processValue;

    // Proportional term
    float p = _kp * error;

    // Derivative term using filtered process value derivative
    float rawDeriv = (processValue - _prevProcessValue) / _dt;
    float alpha = 0.7f;
    float d = alpha * _prevDeriv + (1.0f - alpha) * rawDeriv;
    float dOut = _kd * d;

    // Pre-output from P and D terms
    float output = p + dOut;

    // Integrate only if output is not saturated
    bool saturatedHigh = (output >= _outMax);
    bool saturatedLow = (output <= _outMin);

    if(!saturatedHigh && !saturatedLow) {
        _integral += error * _dt;
        _integral = constrain(_integral, -_integralMax, _integralMax);
    }

    float iOut = _ki * _integral;
    output = p + iOut + dOut;

    // Clamp output to configured bounds
    output = constrain(output, _outMin, _outMax);

    // Apply rate limiting to prevent abrupt throttle changes
    output = constrain(output, _prevOutput - _maxDeltaDown, _prevOutput + _maxDeltaUp);

    // Save state for next iteration
    _prevError = error;
    _prevDeriv = d;
    _prevProcessValue = processValue;
    _prevOutput = output;

    return output;
}
