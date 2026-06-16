#pragma once
#include "constants.h"
#include "adc_utils.h"

/**
 * ADC values for buttons using 5.1K resistor between BUTTONS_PIN and GND:
 * 
 * REJECT_CALL:     0
 * MUTE:            110
 * ANSWER_CALL:     375
 * NEXT_TRACK:      1305
 * PREVIOUS_TRACK:  1715
 * VOLUME_DOWN:     2060
 * VOLUME_UP:       2490
 */
enum Button {
    BUTTON_REJECT_CALL,
    BUTTON_MUTE,
    BUTTON_ANSWER_CALL,
    BUTTON_NEXT_TRACK,
    BUTTON_PREVIOUS_TRACK,
    BUTTON_VOLUME_DOWN,
    BUTTON_VOLUME_UP,
    BUTTON_NONE
};

/**
 * CruiseControl buttons
 */
enum CCButton {
    BUTTONCC_SET,
    BUTTONCC_RESUME,
    BUTTONCC_CANCEL,
    BUTTONCC_NONE
};

namespace Buttons {
    struct ButtonRange {
        int maxValue;
        Button button;
    };

    const ButtonRange buttons[] = {
        {50,    BUTTON_REJECT_CALL},
        {250,   BUTTON_MUTE},
        {800,   BUTTON_ANSWER_CALL},
        {1500,  BUTTON_NEXT_TRACK},
        {1900,  BUTTON_PREVIOUS_TRACK},
        {2200,  BUTTON_VOLUME_DOWN},
        {2600,  BUTTON_VOLUME_UP}
    };

    Button getPressedButton();

    CCButton getPressedCCButton();
}

Button Buttons::getPressedButton() {
    uint16_t rawValue = readADC(BUTTONS_PIN);
    for(const ButtonRange& buttonRange : buttons) {
        if(rawValue <= buttonRange.maxValue) {
            return buttonRange.button;
        }
    }
    return Button::BUTTON_NONE;
}

CCButton Buttons::getPressedCCButton() {
    Button button = Buttons::getPressedButton();
    switch(button) {
        case Button::BUTTON_REJECT_CALL:
        case Button::BUTTON_MUTE:
        case Button::BUTTON_ANSWER_CALL:
        case Button::BUTTON_NEXT_TRACK:
        case Button::BUTTON_PREVIOUS_TRACK:
            return CCButton::BUTTONCC_CANCEL;
        case Button::BUTTON_VOLUME_DOWN:
            return CCButton::BUTTONCC_SET;
        case Button::BUTTON_VOLUME_UP:
            return CCButton::BUTTONCC_RESUME;
        default:
            return CCButton::BUTTONCC_NONE;
    }
}

