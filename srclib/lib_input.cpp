#include "lib_input.h"

namespace ScreenButtonState {
    static uint8_t buttonState = 0;

    void updateButtonState(uint8_t newState) {
        buttonState = buttonState << 4 | newState;
    }

    bool isButtonOn(ScreenButtonId btn) {
        return buttonState >> btn & 1 ? true : false;
    }

    bool wasButtonOn(ScreenButtonId btn) {
        return buttonState >> (4 + btn) & 1? true : false;
    }

    bool isButtonActivated(ScreenButtonId btn) {
        return isButtonOn(btn) && !wasButtonOn(btn);
    }

    bool wasButtonReleased(ScreenButtonId btn) {
        return !isButtonOn(btn) && wasButtonOn(btn);
    }

    bool isAnyButtonOn() {
        return buttonState & 15 ? true : false;
    }

    bool wasAnyButtonOn() {
        return buttonState & 0xf0 ? true : false;
    }

    bool isAnyButtonActivated() {
        return isAnyButtonOn() && !wasAnyButtonOn();
    }

    bool wasAnyButtonReleased() {
        return !isAnyButtonOn() && wasAnyButtonOn();
    }

}

