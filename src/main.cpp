#include <Arduino.h>
#include "can_reader.h"
#include "elm327.h"
#include "indicator_led.h"
#include "throttle.h"
#include "brake_clutch.h"
#include "buttons.h"
#include "pid_controller.h"

CanReader canReader;
Elm327 elm327(canReader);
PIDController cruisePID(PID_KP, PID_KI, PID_KD, 
                        MAIN_LOOP_PERIOD_MS / 1000.0f, 0.0f, 100.0f, 
                        THROTTLE_MAX_DELTA_UP, THROTTLE_MAX_DELTA_DOWN);

void setup() {
    Throttle::init();
    BrakeClutch::init();
    IndicatorLed::init();
    canReader.init();
    elm327.init();
    cruisePID.setIntegralLimit(500.0f);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
