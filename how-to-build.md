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

const char* detectButtonName(uint16_t rawSensorValue) {
    const char* buttonName = "";
    if(rawSensorValue <= 200) {
        buttonName = "Reject call";
    } else if(rawSensorValue <= 400) {
        buttonName = "Mute";
    } else if(rawSensorValue <= 800) {
        buttonName = "Answer call";
    } else if(rawSensorValue <= 1600) {
        buttonName = "Next track";
    } else if(rawSensorValue <= 2000) {
        buttonName = "Previous track";
    } else if(rawSensorValue <= 2300) {
        buttonName = "Volume down";
    } else if(rawSensorValue <= 2700) {
        buttonName = "Volume up";
    } else {
        buttonName = "None";
    }

    return buttonName;
}

void setup() {
    Serial.begin(115200);
}

void loop() {
    uint16_t rawValue = analogRead(GPIO_NUM_35);
    const char* detectedButtonName = detectButtonName(rawValue);
    Serial.printf("Raw value: %4d. Detected button name: %s\n", rawValue, detectedButtonName);
    delay(500);
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
