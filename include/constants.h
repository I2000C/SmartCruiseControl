#pragma once

#include <Arduino.h>

/* Main constants */
#define PRO_CORE_ID 0
#define APP_CORE_ID 1

#define CAN_RX_TASK_PRIORITY 5
#define CAN_PROCESSING_TASK_PRIORITY 4

/* CAN bus constants */
#define CAN_TX_GPIO GPIO_NUM_5
#define CAN_RX_GPIO GPIO_NUM_4
#define CAN_SPEED TWAI_TIMING_CONFIG_500KBITS()
#define CAN_SLEEP_TIME_MS 100
#define CAN_RX_QUEUE_LENGTH 16
#define CAN_PROCESSING_QUEUE_LENGTH 100
#define SNAPSHOT_PERIOD_MS 20
