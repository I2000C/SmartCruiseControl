#include <Arduino.h>
#include "error_codes.h"
#include "can_reader.h"
#include "elm327.h"
#include "indicator_led.h"
#include "throttle.h"
#include "brake_clutch.h"
#include "cruise_control.h"

#define configCHECK_FOR_STACK_OVERFLOW 2

// Stack overflow callback
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("ERROR: Stack overflow in task %s\n", pcTaskName);

    // Disable interrupts
    taskDISABLE_INTERRUPTS();

    // Turn off throttle overriding
    Throttle::enableOverride(false);

    // Restart ESP32
    esp_restart();
}

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

// Fatal error handler: blinks an error code indefinitely using the indicator LED
void fatalErrorLoop(uint8_t code) {
    while(true) {
        for(uint8_t i=0; i<code; i++) {
            IndicatorLed::setState(SystemState::STATE_ACTIVE);
            delay(200);
            IndicatorLed::setState(SystemState::STATE_OFF);
            delay(200);
        }
        delay(1000); // Wait before repeating the error code
    }
}

// Arduino setup() initializes hardware and starts application tasks
void setup() {
    // Initialize status LED
    if(!IndicatorLed::init()) {
        fatalErrorLoop(ERROR_LED_INIT);
    }

    // Initialize throttle outputs and inputs
    if(!Throttle::init()) {
        fatalErrorLoop(ERROR_THROTTLE_INIT);
    }

    // Initialize brake and clutch pins
    BrakeClutch::init();
    
    // Start CAN bus reception and processing
    if(!canReader.init()) {
        fatalErrorLoop(ERROR_CAN_READER_INIT);
    }

    // Start ELM327 serial task
    elm327.init();

    xTaskCreatePinnedToCore(mainTask, "MainTask", 10000, nullptr, MAIN_TASK_PRIORITY, nullptr, APP_CORE_ID);
}

// Arduino loop() is idle because the app uses FreeRTOS tasks
void loop() {
    vTaskDelay(portMAX_DELAY);
}
