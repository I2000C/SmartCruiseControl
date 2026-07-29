#include "buttons.h"
#include "adc_utils.h"

static Button lastCandidate = Button::BUTTON_NONE;
static uint8_t consecutiveReads = 0;

Button Buttons::classify(uint16_t rawValue) {
    for(const ButtonRange& buttonRange : buttons) {
        if(rawValue <= buttonRange.maxValue) {
            return buttonRange.button;
        }
    }
    return Button::BUTTON_NONE;
}

Button Buttons::getPressedButton() {
    uint16_t rawValue = ADC::readFiltered(BUTTONS_PIN);
    Button current = classify(rawValue);

    // No button detected
    if(current == Button::BUTTON_NONE) {
        lastCandidate = Button::BUTTON_NONE;
        return Button::BUTTON_NONE;
    }

    // Candidate changed
    if(current != lastCandidate) {
        lastCandidate = current;
        consecutiveReads = 0;
        return Button::BUTTON_NONE;
    }

    // Confirm min consecutive reads
    if(consecutiveReads < BUTTONS_MIN_CONSECUTIVE_READINGS) {
        consecutiveReads++;
        return Button::BUTTON_NONE;
    }

    return current;
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
