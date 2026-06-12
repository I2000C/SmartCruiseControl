#include <Arduino.h>
#include "can_reader.h"

CanReader canReader;

void setup() {
    canReader.init();
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
