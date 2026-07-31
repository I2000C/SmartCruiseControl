# How to build

## 1. VSCode and PlatformIO installation

You have to download and intall VSCode (https://code.visualstudio.com/) and then install the PlatformIO extension

## 2. Button key calibration

Once you have both VSCode and PlatformIO, you'll need to calibrate button keys because the values could differ from the ones present in the code.

To do that, create a new project in PlatformIO for the ESP32 board.

Then follow these steps:
1. Paste the next code in the `platformio.ini` file

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
upload_speed = 921600
```

2. Paste the next code in the `main.cpp` file:

```cpp
#include <Arduino.h>
#include <cinttypes>

#define LED_PIN GPIO_NUM_2
#define BUTTONS_PIN GPIO_NUM_35

uint16_t minValue = 0xFFFF;
uint16_t maxValue = 0;
uint32_t sumValue = 0;
uint16_t numSamples = 0;

void resetValues() {
    minValue = 0xFFFF;
    maxValue = 0;
    sumValue = 0;
    numSamples = 0;
}

inline uint16_t readFiltered(gpio_num_t pin, uint8_t samples = 16, uint8_t samplesToDiscard = 2) {
    uint16_t adcSamples[samples];

    if(samplesToDiscard * 2 >= samples) {
        samplesToDiscard = 0;
    }

    // Adquire ADC samples
    for(int i=0; i<samples; i++) {
        adcSamples[i] = analogRead(pin);
    }

    // Sort samples using insertion sort
    for(int index=1; index<samples; index++) {
        uint16_t currentSample = adcSamples[index];
        int8_t prevIndex = index - 1;

        while(prevIndex >= 0 && adcSamples[prevIndex] > currentSample) {
            adcSamples[prevIndex + 1] = adcSamples[prevIndex];
            prevIndex--;
        }

        adcSamples[prevIndex + 1] = currentSample;
    }

    // Compute the average after discarding extreme values
    uint32_t sum = 0;
    for(uint8_t i=samplesToDiscard; i<samples-samplesToDiscard; i++) {
        sum += adcSamples[i];
    }

    uint8_t validSamples = samples - (2 * samplesToDiscard);
    return sum / validSamples;
}

void detectButton(uint16_t rawSensorValue) {
    const char* buttonName = "";
    bool showRawValue = true;
    if(rawSensorValue <= 50) {
        buttonName = "Reject call";
        digitalWrite(LED_PIN, HIGH);
    } else if(rawSensorValue <= 250) {
        buttonName = "Mute";
        digitalWrite(LED_PIN, HIGH);
    } else if(rawSensorValue <= 800) {
        buttonName = "Answer call";
        digitalWrite(LED_PIN, HIGH);
    } else if(rawSensorValue <= 1600) {
        buttonName = "Next track";
        digitalWrite(LED_PIN, HIGH);
    } else if(rawSensorValue <= 1900) {
        buttonName = "Previous track";
        digitalWrite(LED_PIN, HIGH);
    } else if(rawSensorValue <= 2300) {
        buttonName = "Volume down";
        digitalWrite(LED_PIN, HIGH);
    } else if(rawSensorValue <= 2700) {
        buttonName = "Volume up";
        digitalWrite(LED_PIN, HIGH);
    } else {
        buttonName = "None";
        showRawValue = false;
        resetValues();
        digitalWrite(LED_PIN, LOW);
    }

    if(showRawValue) {
        numSamples++;
        sumValue += rawSensorValue;

        if(rawSensorValue < minValue) {
            minValue = rawSensorValue;
        }
        if(rawSensorValue > maxValue) {
            maxValue = rawSensorValue;
        }

        uint16_t averageValue = sumValue / numSamples;
        Serial.printf("Raw value: %4d (min: %4d, max: %4d, avg: %4d). Detected button name: %s\n", rawSensorValue, minValue, maxValue, averageValue, buttonName);
    }
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
}

void loop() {
    uint16_t rawValue = readFiltered(BUTTONS_PIN);
    detectButton(rawValue);
    delay(100);
}
```

3. Flash the code to the ESP32 using the **Upload button** while the ESP32 is connected to the computer (Don't forget to press the BOOT button until flashing starts)
4. Connect the **GND wire** of the button set to **GND**
5. Connect the **KEY wire** of the button set to **GPIO 35**
6. Connect a **5k resistor** between **GPIO 35** and **3.3V**
7. Open the serial monitor in the computer while you press each button and check if read value is in the correct range and detected button is correct
8. If one or more buttons don't match, **you'll have to modify** the buttons module of the project

## 3.Downloading and flashing the project

To download the project, go to a folder, open a terminal and type `git clone https://github.com/I2000C/SmartCruiseControl.git --recurse-submodules`

Then open project folder in VSCode and flash it to the ESP32 using the **Upload button** while the ESP32 is connected to the computer (Don't forget to press the BOOT button until flashing starts)

## 4. Final steps

Before ordering the final board, I recomend that you test the project using one or more breadboards.

Once you get the project working, you can order the final board by generating the required gerber files from the kicad project (you'll need to install Kicad to do this) and then send these files to your favourite board supplier (e.g.: PCBWay, JLCPCB, ...)
