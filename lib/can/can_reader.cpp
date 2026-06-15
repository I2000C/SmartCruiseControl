#include "constants.h"
#include "can_reader.h"
#include "can_data.h"
#include "can_decoder.h"
#include <driver/twai.h>

void canProcessingTask(void* args) {
    if(args == nullptr) {
        return;
    }
    CanReader* canReader = (CanReader*) args;

    CanFrame frame;
    uint32_t lastSpeedUpdate = millis();

    while(true) {
        if(xQueueReceive(canReader->canQueue, &frame, pdMS_TO_TICKS(REFRESH_COMPUTED_DATA_TIME_MS))) {
            canReader->canDecoder.decodeFrame(frame, canReader->sharedState);
        }

        if(millis() - lastSpeedUpdate > REFRESH_COMPUTED_DATA_TIME_MS) {
            canReader->canDecoder.refreshComputedData(canReader->sharedState);
        }
    }
}

void canRxTask(void* args) {
    if(args == nullptr) {
        return;
    }
    CanReader* canReader = (CanReader*) args;

    twai_message_t message;
    CanFrame frame;

    while(true) {
        if(twai_receive(&message, pdMS_TO_TICKS(CAN_SLEEP_TIME_MS)) == ESP_OK) {
            frame.id = message.identifier;
            frame.dlc = message.data_length_code;
            frame.timestamp = millis();

            size_t len = frame.dlc < 8 ? frame.dlc : 8;
            memcpy(frame.data, message.data, len);

            // Avoid blocking
            xQueueSend(canReader->canQueue, &frame, 0);
        }
    }
}

void CanReader::readState(VehicleState& state) const {
    // Seq lock
    uint32_t s1, s2;
    do {
        s1 = sharedState.seq.load(std::memory_order_acquire);
        state = sharedState.state;
        s2 = sharedState.seq.load(std::memory_order_acquire);
    } while((s1 != s2) || (s1 & 1));
}

bool CanReader::init() {
    // Main can bus config
    twai_general_config_t generalConfig = {
        .mode = TWAI_MODE_LISTEN_ONLY,
        .tx_io = CAN_TX_GPIO,
        .rx_io = CAN_RX_GPIO,
        .clkout_io = TWAI_IO_UNUSED,
        .bus_off_io = TWAI_IO_UNUSED,
        .tx_queue_len = 0,
        .rx_queue_len = CAN_RX_QUEUE_LENGTH,
        .clkout_divider = 0
    };

    // CAN bus speed
    twai_timing_config_t timingConfig = CAN_SPEED;

    // CAN message filter: Accept all messages
    twai_filter_config_t filterConfig = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    // Init driver
    if(twai_driver_install(&generalConfig, &timingConfig, &filterConfig) != ESP_OK) {
        return false;
    }

    if(twai_start() != ESP_OK) {
        return false;
    }

    sharedState.state.startDistance = -1.0f;
    canQueue = xQueueCreate(CAN_PROCESSING_QUEUE_LENGTH, sizeof(CanFrame));

    xTaskCreatePinnedToCore(canRxTask, "CanTask", 5000, this, CAN_RX_TASK_PRIORITY, nullptr, PRO_CORE_ID);
    xTaskCreatePinnedToCore(canProcessingTask, "CanProcessingTask", 10000, this, CAN_PROCESSING_TASK_PRIORITY, nullptr, PRO_CORE_ID);

    return true;
}
