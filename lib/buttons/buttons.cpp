#include "buttons.h"

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
            return CCButton::BUTTON_CANCEL;
        case Button::BUTTON_VOLUME_DOWN:
            return CCButton::BUTTON_SET;
        case Button::BUTTON_VOLUME_UP:
            return CCButton::BUTTON_RESUME;
        default:
            return CCButton::BUTTON_NONE;
    }
}
