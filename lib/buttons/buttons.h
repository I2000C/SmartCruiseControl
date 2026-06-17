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
enum class Button {
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
enum class CCButton {
    BUTTON_SET,
    BUTTON_RESUME,
    BUTTON_CANCEL,
    BUTTON_NONE
};

namespace Buttons {
    struct ButtonRange {
        int maxValue;
        Button button;
    };

    const ButtonRange buttons[] = {
        {50,    Button::BUTTON_REJECT_CALL},
        {250,   Button::BUTTON_MUTE},
        {800,   Button::BUTTON_ANSWER_CALL},
        {1500,  Button::BUTTON_NEXT_TRACK},
        {1900,  Button::BUTTON_PREVIOUS_TRACK},
        {2200,  Button::BUTTON_VOLUME_DOWN},
        {2600,  Button::BUTTON_VOLUME_UP}
    };

    Button getPressedButton();

    CCButton getPressedCCButton();
}
