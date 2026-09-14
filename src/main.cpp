#include <Arduino.h>
#include "error_codes.h"
#include "can_reader.h"
#include "elm327.h"
#include "indicator_led.h"
#include "throttle.h"
#include "brake_clutch.h"
#include "cruise_control.h"
#include "debug.h"

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

    const TickType_t period = pdMS_TO_TICKS(BUTTONS_QUERY_PERIOD_MS);
    TickType_t lastWakeTime = xTaskGetTickCount();

    uint8_t pidUpdateInterval = MAIN_LOOP_PERIOD_MS / BUTTONS_QUERY_PERIOD_MS;
    uint8_t counter = 0;

    while(true) {
        CCButton pressedButton = Buttons::getPressedCCButton();

        if(++counter >= pidUpdateInterval) {
            counter = 0;
            canReader.readState(vehicleState);                  // Read the latest vehicle state from CAN
            cruiseControl.loop(vehicleState, pressedButton);    // Run cruise control logic
        }

        vTaskDelayUntil(&lastWakeTime, period); // Sleep until next loop period
    }
}

// Test buttons and pedals
void testButtonsAndPedals() {
    VehicleState vehicleState;

    while(1) {
        bool turnOnLed = false;

        // Check pressed button
        Button button = Buttons::getPressedButton();
        switch(button) {
            case Button::BUTTON_REJECT_CALL:
                Serial.println("Button pressed: BUTTON_REJECT_CALL");
                turnOnLed = true;
                break;
            case Button::BUTTON_MUTE:
                Serial.println("Button pressed: BUTTON_MUTE");
                turnOnLed = true;
                break;
            case Button::BUTTON_ANSWER_CALL:
                Serial.println("Button pressed: BUTTON_ANSWER_CALL");
                turnOnLed = true;
                break;
            case Button::BUTTON_NEXT_TRACK:
                Serial.println("Button pressed: BUTTON_NEXT_TRACK");
                turnOnLed = true;
                break;
            case Button::BUTTON_PREVIOUS_TRACK:
                Serial.println("Button pressed: BUTTON_PREVIOUS_TRACK");
                turnOnLed = true;
                break;
            case Button::BUTTON_VOLUME_DOWN:
                Serial.println("Button pressed: BUTTON_VOLUME_DOWN");
                turnOnLed = true;
                break;
            case Button::BUTTON_VOLUME_UP:
                Serial.println("Button pressed: BUTTON_VOLUME_UP");
                turnOnLed = true;
                break;
        }

        // Check brake pedal
        if(BrakeClutch::isBrakePressed()) {
            Serial.println("Pedal pressed: BRAKE");
            turnOnLed = true;
        }

        // Check clutch pedal
        if(BrakeClutch::isClutchPressed()) {
            Serial.println("Pedal pressed: CLUTCH");
            turnOnLed = true;
        }

        // Read vehicle state
        canReader.readState(vehicleState);

        // Check parking brake
        if(vehicleState.parkingBrake) {
            Serial.println("Lever pressed: PARKING_BRAKE");
            turnOnLed = true;
        }

        // Check reverse light
        if(vehicleState.reverseLight) {
            Serial.println("Light activated: REVERSE_LIGHT");
            turnOnLed = true;
        }

        if(turnOnLed) {
            IndicatorLed::setState(SystemState::STATE_ACTIVE);
        } else {
            IndicatorLed::setState(SystemState::STATE_OFF);
        }

        delay(MAIN_LOOP_PERIOD_MS);
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

    IndicatorLed::setState(SystemState::STATE_ACTIVE);

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

    // Wait a second for a button to be pressed to enter debug mode
    Debug::setEnabled(false);
    uint32_t buttonDebugTime = 0;
    while(buttonDebugTime < 1000) {
        Button button = Buttons::getPressedButton();
        if(button == Button::BUTTON_NONE) {
            delay(BUTTONS_QUERY_PERIOD_MS);
            buttonDebugTime += BUTTONS_QUERY_PERIOD_MS;
        } else {
            if(button == Button::BUTTON_MUTE) {
                // Enable button and pedals testing mode
                Debug::setEnabled(true);
                Serial.begin(SERIAL_DEBUG_BAUDRATE);
                Debug::println("Buttons debug mode is enabled");

                // Fade indicator LED for 4 seconds
                for(uint32_t time = 0; time < 4000; time += MAIN_LOOP_PERIOD_MS) {
                    IndicatorLed::setState(SystemState::STATE_OVERRIDE);
                    delay(MAIN_LOOP_PERIOD_MS);
                }

                IndicatorLed::setState(SystemState::STATE_OFF);
                testButtonsAndPedals();
            } else {
                // Enable debug mode
                Debug::setEnabled(true);
                Serial.begin(SERIAL_DEBUG_BAUDRATE);
                Debug::println("Debug mode is enabled");
    
                // Fade indicator LED for 2 seconds
                for(uint32_t time = 0; time < 2000; time += MAIN_LOOP_PERIOD_MS) {
                    IndicatorLed::setState(SystemState::STATE_OVERRIDE);
                    delay(MAIN_LOOP_PERIOD_MS);
                }
                break;
            }
        }
    }

    // Start ELM327 serial task if debug mode is not enabled
    if(!Debug::isEnabled()) {
        Serial.begin(SERIAL_ELM327_BAUDRATE);
        elm327.init();
    }

    IndicatorLed::setState(SystemState::STATE_OFF);

    xTaskCreatePinnedToCore(mainTask, "MainTask", 10000, nullptr, MAIN_TASK_PRIORITY, nullptr, APP_CORE_ID);
}

// Arduino loop() is idle because the app uses FreeRTOS tasks
void loop() {
    vTaskDelay(portMAX_DELAY);
}
