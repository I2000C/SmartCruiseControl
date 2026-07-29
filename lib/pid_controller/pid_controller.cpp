#include "pid_controller.h"
#include "constants.h"
#include <Arduino.h>
#include <cmath>

void PIDController::setTunings(float kp, float ki, float kd) {
    // Update controller gains
    _kp = kp;
    _ki = ki;
    _kd = kd;
    if(_ki > 0.0f) {
        _integralMax = MAX_INTEGRAL_CONTRIBUTION / _ki;
    } else {
        _integralMax = 0.0f;
    }
}

void PIDController::reset() {
    // Clear historic PID state
    _integral = 0.0f;
    _prevDeriv = 0.0f;
    _prevProcessValue = 0.0f;
    _prevOutput = 0.0f;
    _prevError = 0.0f;
    _firstRun = true;
}

float PIDController::compute(float setpoint, float processValue) {
    // Initialize previous process value on first execution
    if(_firstRun) {
        _prevProcessValue = processValue;
        _firstRun = false;
    }

    // Compute control error
    float error = setpoint - processValue;

    // Apply dead zone
    if(fabsf(error) < ERROR_DEADZONE) {
        error = 0.0f;
    }

    // Proportional term
    float pOut = _kp * error;

    // Derivative term (computed from the process value with a small filter)
    float rawDeriv = (processValue - _prevProcessValue) / _dt;
    float deriv = DERIVATIVE_ALPHA * _prevDeriv + (1.0f - DERIVATIVE_ALPHA) * rawDeriv;
    float dOut = -_kd * deriv;

    // Integrate error
    _integral += error * _dt;
    _integral = constrain(_integral, -_integralMax, _integralMax);

    // Integral term
    float iOut = _ki * _integral;

    // Controller output before any limiting
    float unsatOutput = pOut + iOut + dOut;

    // Apply output saturation
    float saturatedOutput = constrain(unsatOutput, _outMin, _outMax);

    // Anti-windup (back-calculation)
    _integral += ANTI_WINDUP_GAIN * (saturatedOutput - unsatOutput) * _dt;
    _integral = constrain(_integral, -_integralMax, _integralMax);

    // Apply slew-rate limiting
    float output = constrain(saturatedOutput, _prevOutput - _maxDeltaDown, _prevOutput + _maxDeltaUp);

    // Save controller state
    _prevDeriv = deriv;
    _prevProcessValue = processValue;
    _prevOutput = output;
    _prevError = error;

    return output;
}
