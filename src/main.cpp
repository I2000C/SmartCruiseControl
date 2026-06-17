#include <Arduino.h>
#include "can_reader.h"
#include "elm327.h"
#include "indicator_led.h"
#include "throttle.h"
#include "brake_clutch.h"
#include "cruise_control.h"

static CanReader canReader;
static Elm327 elm327(canReader);

void mainTask(void* args) {
    VehicleState vehicleState;
    CruiseControl cruiseControl;

    const TickType_t period = pdMS_TO_TICKS(MAIN_LOOP_PERIOD_MS);
    TickType_t lastWakeTime = xTaskGetTickCount();

    while(true) {
        canReader.readState(vehicleState);
        cruiseControl.loop(vehicleState);
        vTaskDelayUntil(&lastWakeTime, period);
    }
}

void setup() {
    Throttle::init();
    BrakeClutch::init();
    IndicatorLed::init();
    canReader.init();
    elm327.init();

    xTaskCreatePinnedToCore(mainTask, "MainTask", 10000, nullptr, MAIN_TASK_PRIORITY, nullptr, APP_CORE_ID);
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
