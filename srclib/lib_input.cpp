#include "lib_input.h"


uint8_t ScreenButtonState::buttonState;

void ScreenButtonState::updateButtonState(uint8_t newState) {
    buttonState = buttonState << 4 | newState;
}

bool ScreenButtonState::isButtonOn(ScreenButtonId btn) {
    return buttonState >> btn & 1 ? true : false;
}

bool ScreenButtonState::wasButtonOn(ScreenButtonId btn) {
    return buttonState >> (4 + btn) & 1? true : false;
}

bool ScreenButtonState::isButtonActivated(ScreenButtonId btn) {
    return isButtonOn(btn) && !wasButtonOn(btn);
}

bool ScreenButtonState::wasButtonReleased(ScreenButtonId btn) {
    return !isButtonOn(btn) && wasButtonOn(btn);
}

bool ScreenButtonState::isAnyButtonOn() {
    return buttonState & 15 ? true : false;
}

bool ScreenButtonState::wasAnyButtonOn() {
    return buttonState & 0xf0 ? true : false;
}

bool ScreenButtonState::isAnyButtonActivated() {
    return isAnyButtonOn() && !wasAnyButtonOn();
}

bool ScreenButtonState::wasAnyButtonReleased() {
    return !isAnyButtonOn() && wasAnyButtonOn();
}
