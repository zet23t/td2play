#ifndef __LIB_INPUT_H__
#define __LIB_INPUT_H__

#include <inttypes.h>
#include "lib_FixedMath.h"

#define TinyArcadePinX 42
#define TinyArcadePinY 1
#define TinyArcadePin1 45
#define TinyArcadePin2 44

#define TinyArcadePinUp 42
#define TinyArcadePinDown 19
#define TinyArcadePinLeft 25
#define TinyArcadePinRight 15

enum ScreenButtonId {
    SCREENBUTTON_BOTTOMLEFT = 0,
    SCREENBUTTON_TOPLEFT = 1,
    SCREENBUTTON_TOPRIGHT = 2,
    SCREENBUTTON_BOTTOMRIGHT = 3
};

namespace ScreenButtonState {
    void updateButtonState(uint8_t newState);
    bool isButtonOn(ScreenButtonId btn);
    bool wasButtonOn(ScreenButtonId btn);
    bool isButtonActivated(ScreenButtonId btn);
    bool wasButtonReleased(ScreenButtonId btn);
    bool isAnyButtonOn();
    bool wasAnyButtonOn();
    bool isAnyButtonActivated();
    bool wasAnyButtonReleased();
};

namespace Joystick {
    namespace Phase {
        extern const uint8_t CURRENT;
        extern const uint8_t PREVIOUS;
    }
    void updateJoystick();
    Fixed2D4 getJoystick();
    bool getButton(int);
    bool getButton(int,int);
}


#endif // __LIB_INPUT_H__
