#include "pid_controller.h"
#include <Arduino.h>

void PIDController::setTunings(float kp, float ki, float kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

void PIDController::setIntegralLimit(float limit) {
    _integralMax = limit;
}

void PIDController::reset() {
    _integral = 0.0f;
    _prevError = 0.0f;
    _prevDeriv = 0.0f;
    _prevProcessValue = 0.0f;
    _prevOutput = 0.0f;
}

float PIDController::compute(float setpoint, float processValue) {
    float error = setpoint - processValue;

    // =========================
    // P
    // =========================
    float p = _kp * error;

    // =========================
    // D (filtered)
    // =========================
    float rawDeriv = (processValue - _prevProcessValue) / _dt;

    // simple exponential filter
    float alpha = 0.7f;
    float d = alpha * _prevDeriv + (1.0f - alpha) * rawDeriv;

    float dOut = _kd * d;

    // =========================
    // pre-output
    // =========================
    float output = p + dOut;

    // =========================
    // anti-windup (only integrate if NOT saturated)
    // =========================
    bool saturatedHigh = (output >= _outMax);
    bool saturatedLow = (output <= _outMin);

    if(!saturatedHigh && !saturatedLow) {
        _integral += error * _dt;
        _integral = constrain(_integral, -_integralMax, _integralMax);
    }

    float iOut = _ki * _integral;

    output = p + iOut + dOut;

    // clamp
    output = constrain(output, _outMin, _outMax);

    // rate limit
    output = constrain(output, _prevOutput - _maxDeltaDown, _prevOutput + _maxDeltaUp);

    // save state
    _prevError = error;
    _prevDeriv = d;
    _prevProcessValue = processValue;
    _prevOutput = output;

    return output;
}
