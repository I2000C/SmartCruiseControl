#pragma once

#include <Arduino.h>

/* Main core and task constants */
#define PRO_CORE_ID 0                // ESP32 core used for CAN tasks
#define APP_CORE_ID 1                // ESP32 core used for application tasks

#define CAN_RX_TASK_PRIORITY 5       // Priority for CAN receive task
#define CAN_PROCESSING_TASK_PRIORITY 4 // Priority for CAN decoding task
#define ELM327_TASK_PRIORITY 1       // Priority for ELM327 serial task
#define MAIN_TASK_PRIORITY 2         // Priority for main cruise control loop task

#define MAIN_LOOP_PERIOD_MS 50        // Main control loop period in milliseconds

/* Cruise control limits and thresholds */
#define MAX_TARGET_SPEED_DIFF 25     // Maximum resume speed difference in km/h
#define MIN_SPEED_KMH 25             // Minimum allowed cruise speed
#define MAX_SPEED_KMH 135            // Maximum allowed cruise speed

#define MIN_CAR_SPEED_KMH (MIN_SPEED_KMH-5)  // Minimum allowed car speed
#define MAX_CAR_SPEED_KMH (MAX_SPEED_KMH+5)  // Maximum allowed car speed
#define MIN_RPM 1200                 // Minimum engine RPM for cruise control
#define MAX_RPM 3500                 // Maximum engine RPM for cruise control

#define THROTTLE_PEDAL_THRESHOLD_ENABLE 20   // Pedal percent threshold to bypass throttle override
#define THROTTLE_PEDAL_THRESHOLD_DISABLE 10  // Pedal percent threshold to re-enable throttle override

#define SPEED_TIMEOUT_MS 3000         // Maximum age of speed data before invalidation
#define RPM_TIMEOUT_MS 3000           // Maximum age of RPM data before invalidation

/* PIDController constants */
#define PID_KP 0.6f                   // Proportional gain
#define PID_KI 0.2f                   // Integral gain
#define PID_KD 0.05f                  // Derivative gain
#define THROTTLE_RATE_UP_PER_SEC 0.15f   // Maximum throttle increase per second (15 % by default)
#define THROTTLE_RATE_DOWN_PER_SEC 0.40f // Maximum throttle decrease per second (40 % by default)
#define THROTTLE_MAX_DELTA_UP (THROTTLE_RATE_UP_PER_SEC * (MAIN_LOOP_PERIOD_MS / 1000.0f))
#define THROTTLE_MAX_DELTA_DOWN (THROTTLE_RATE_DOWN_PER_SEC * (MAIN_LOOP_PERIOD_MS / 1000.0f))

#define DERIVATIVE_ALPHA 0.7f           // Alpha value to smooth derivative
#define MAX_INTEGRAL_CONTRIBUTION 0.2f  // Max integral contribution to PID (20 % by default)
#define ANTI_WINDUP_GAIN 0.5f           // Anti windup gain
#define ERROR_DEADZONE 0.3f             // Dead zone for error values

/* Indicator LED constants */
#define INDICATOR_LED_PIN GPIO_NUM_25           // Status LED pin for cruise control state
#define INDICATOR_LED_PWM_CHANNEL 0             // PWM channel for indicator LED
#define INDICATOR_LED_BUILTIN_PIN GPIO_NUM_2    // Same as status LED but using internal LED
#define INDICATOR_LED_BUILTIN_PWM_CHANNEL 1     // PWM channel for builtin indicator LED
#define INDICATOR_LED_PWM_FREQ_HZ 1000          // PWM frequency for indicator LED
#define INDICATOR_LED_PWM_RESOLUTION_BITS 8     // PWM resolution bits for indicator LED
#define INDICATOR_LED_PWM_RESOLUTION (1<<INDICATOR_LED_PWM_RESOLUTION_BITS)
#define INDICATOR_LED_PWM_MAX (INDICATOR_LED_PWM_RESOLUTION-1)

/* Throttle output and input constants */
#define THROTTLE_APPS1_PIN GPIO_NUM_18            // PWM output for simulated APP sensor 1
#define THROTTLE_APPS2_PIN GPIO_NUM_19            // PWM output for simulated APP sensor 2
#define THROTTLE_APPS1_PWM_CHANNEL 2              // PWM channel for APP1 output
#define THROTTLE_APPS2_PWM_CHANNEL 3              // PWM channel for APP2 output
#define THROTTLE_PWM_FREQ_HZ 19000                // PWM frequency for throttle outputs
#define THROTTLE_PWM_RESOLUTION_BITS 12           // PWM resolution bits for throttle outputs
#define THROTTLE_PWM_RESOLUTION (1<<THROTTLE_PWM_RESOLUTION_BITS)
#define THROTTLE_PWM_MAX (THROTTLE_PWM_RESOLUTION-1)

#define THROTTLE_APPS2_PEDAL_PIN GPIO_NUM_34      // Analog input from actual accelerator pedal

#define THROTTLE_RELAY_PIN GPIO_NUM_21            // Relay control pin for throttle override

/* Brake and clutch constants */
#define BRAKE_PIN GPIO_NUM_22                     // Brake pedal pin      
#define CLUTCH_PIN GPIO_NUM_23                    // Clutch pedal pin

/* Buttons */
#define BUTTONS_PIN GPIO_NUM_35                   // Analog pin for button matrix readout
#define BUTTONS_MIN_CONSECUTIVE_READINGS 3        // Amount of consecutive readings to make sure a button has truly be pressed
#define BUTTONS_QUERY_PERIOD_MS 10                // Buttons query period in milliseconds.
                                                  // BUTTONS_QUERY_PERIOD_MS < MAIN_LOOP_PERIOD_MS and MAIN_LOOP_PERIOD_MS % BUTTONS_QUERY_PERIOD_MS == 0

#define BUTTONS_REPEAT_DELAY_MS 500               // Initial button delay to repeat action. BUTTONS_INITIAL_REPEAT_DELAY_MS % MAIN_LOOP_PERIOD_MS == 0
#define BUTTONS_REPEAT_PERIOD_MS 200              // Period to repeat button action. BUTTONS_REPEAT_PERIOD_MS % MAIN_LOOP_PERIOD_MS == 0
inline constexpr uint16_t BUTTONS_REPEAT_DELAY_TICKS = BUTTONS_REPEAT_DELAY_MS / MAIN_LOOP_PERIOD_MS;
inline constexpr uint16_t BUTTONS_REPEAT_PERIOD_TICKS = BUTTONS_REPEAT_PERIOD_MS / MAIN_LOOP_PERIOD_MS;

/* CAN bus constants */
#define CAN_TX_GPIO GPIO_NUM_5                    // CAN bus transmit pin
#define CAN_RX_GPIO GPIO_NUM_4                    // CAN bus receive pin
#define CAN_SPEED TWAI_TIMING_CONFIG_500KBITS()   // CAN bus bitrate configuration
#define CAN_SLEEP_TIME_MS 100                     // Max wait time for CAN receive
#define CAN_RX_QUEUE_LENGTH 16                    // Queue length for raw CAN frames
#define CAN_PROCESSING_QUEUE_LENGTH 100           // Queue length for decoded CAN processing
#define REFRESH_COMPUTED_DATA_TIME_MS 100         // Interval to refresh computed vehicle data

/* Serial constants */
#define SERIAL_BAUDRATE 115200                    // Serial baud rate for ELM327 / debug serial
