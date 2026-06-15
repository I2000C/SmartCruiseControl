#include <Arduino.h>
#include "can_reader.h"
#include "elm327.h"
#include "indicator_led.h"
#include "throttle.h"

CanReader canReader;
Elm327 elm327(canReader);

void setup() {
    Throttle::init();
    IndicatorLed::init();
    canReader.init();
    elm327.init();
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
