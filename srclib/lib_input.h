#ifndef __LIB_INPUT_H__
#define __LIB_INPUT_H__

#include <inttypes.h>

enum ScreenButtonId {
    SCREENBUTTON_BOTTOMLEFT = 0,
    SCREENBUTTON_TOPLEFT = 1,
    SCREENBUTTON_TOPRIGHT = 2,
    SCREENBUTTON_BOTTOMRIGHT = 3
};

class ScreenButtonState {
private:
    static uint8_t buttonState;
public:
    static void updateButtonState(uint8_t newState);
    static bool isButtonOn(ScreenButtonId btn);
    static bool wasButtonOn(ScreenButtonId btn);
    static bool isButtonActivated(ScreenButtonId btn);
    static bool wasButtonReleased(ScreenButtonId btn);
    static bool isAnyButtonOn();
    static bool wasAnyButtonOn();
    static bool isAnyButtonActivated();
    static bool wasAnyButtonReleased();
};


#endif // __LIB_INPUT_H__
