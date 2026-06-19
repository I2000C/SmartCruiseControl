#include <Arduino.h>
#include "can_reader.h"
#include "elm327.h"
#include "indicator_led.h"
#include "throttle.h"
#include "cruise_control.h"

static CanReader canReader;                  // Shared CAN reader instance
static Elm327 elm327(canReader);             // ELM327 emulator tied to CAN reader

// Main application task that runs the cruise control loop periodically
void mainTask(void* args) {
    VehicleState vehicleState;
    CruiseControl cruiseControl;

    const TickType_t period = pdMS_TO_TICKS(MAIN_LOOP_PERIOD_MS);
    TickType_t lastWakeTime = xTaskGetTickCount();

    while(true) {
        canReader.readState(vehicleState);      // Read the latest vehicle state from CAN
        cruiseControl.loop(vehicleState);       // Run cruise control logic
        vTaskDelayUntil(&lastWakeTime, period); // Sleep until next loop period
    }
}

// Arduino setup() initializes hardware and starts application tasks
void setup() {
    Throttle::init();                         // Initialize throttle outputs and inputs
    IndicatorLed::init();                     // Initialize status LED
    canReader.init();                         // Start CAN bus reception and processing
    elm327.init();                            // Start ELM327 serial task

    xTaskCreatePinnedToCore(mainTask, "MainTask", 10000, nullptr, MAIN_TASK_PRIORITY, nullptr, APP_CORE_ID);
}

// Arduino loop() is idle because the app uses FreeRTOS tasks
void loop() {
    vTaskDelay(portMAX_DELAY);
}