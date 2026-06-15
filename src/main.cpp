#include <Arduino.h>
#include "can_reader.h"
#include "elm327.h"
#include "indicator_led.h"

CanReader canReader;
Elm327 elm327(canReader);

void setup() {
    canReader.init();
    elm327.init();
    IndicatorLed::init();
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
