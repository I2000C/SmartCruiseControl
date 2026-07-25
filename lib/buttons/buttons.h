#pragma once
#include "constants.h"

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
 * Cruise control button mapping from vehicle button bank
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

    inline constexpr ButtonRange buttons[] = {
        {200,    Button::BUTTON_REJECT_CALL},
        {400,   Button::BUTTON_MUTE},
        {800,   Button::BUTTON_ANSWER_CALL},
        {1600,  Button::BUTTON_NEXT_TRACK},
        {2000,  Button::BUTTON_PREVIOUS_TRACK},
        {2300,  Button::BUTTON_VOLUME_DOWN},
        {2700,  Button::BUTTON_VOLUME_UP}
    };

    // Determine button identity from raw ADC value
    Button classify(uint16_t rawValue);

    // Sample the button ADC pin and return the pressed button if stable
    Button getPressedButton();

    // Map generic button presses to cruise control commands
    CCButton getPressedCCButton();
}
