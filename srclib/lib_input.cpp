#include "lib_input.h"
#include <stdio.h>
#ifndef WIN32
#include <SPI.h>
#include <Wire.h>
#else

int analogRead(int);
int digitalRead(int);

#endif // WIN32

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


namespace Joystick {
    namespace Phase {
        const uint8_t CURRENT = 0;
        const uint8_t PREVIOUS = 1;
    }
    static Fixed2D4 position;
    static char buttonA,buttonB;
    static uint8_t arcadeVersion = 1;

    static int clampAndApplyDeadZone5bitTo4bit(int x) {
        // eliminate center zone values first
        if (x < 12) x += 4;
        else if (x >= 20) x -= 4;
        else x = 15;
        // value is now [4..27] with 15 == center
        if (x < 8) x = 8;
        else if (x >= 24) x = 23;
        // make it a 16bit value
        return x - 8;
    }

    void updateJoystick() {
        static bool init = false;
        if (!init) {
            init = true;
            #ifndef WIN32
            // if this isn't configured, the TS+ doesn't properly boot and needs a
            // special start holding a button during power up.
            pinMode(TinyArcadePin1, INPUT_PULLUP);
            pinMode(TinyArcadePin2, INPUT_PULLUP);
            pinMode(4, INPUT_PULLUP);
            if (!digitalRead(4)) {
                arcadeVersion = 2;
            }
            if (arcadeVersion == 2) {
                pinMode(TinyArcadePinUp, INPUT_PULLUP);
                pinMode(TinyArcadePinDown, INPUT_PULLUP);
                pinMode(TinyArcadePinLeft, INPUT_PULLUP);
                pinMode(TinyArcadePinRight, INPUT_PULLUP);
            }
            analogWrite(A0, analogRead(A0));
            #endif // WIN32
        }
        if (arcadeVersion == 1) {
            // reading values from analog isn't very accurate; 4bit precision is enough I guess
            FixedNumber16<4> x(0, clampAndApplyDeadZone5bitTo4bit(analogRead(TinyArcadePinX) >> 5));
            FixedNumber16<4> y(0, clampAndApplyDeadZone5bitTo4bit(analogRead(TinyArcadePinY) >> 5));
            FixedNumber16<4> half(0,7);
            x = half - x;
            y = y - half;
            position.x = x;
            position.y = y;
            buttonA = buttonA<<1 | (digitalRead(TinyArcadePin1) ? 0 : 1);
            buttonB = buttonB<<1 | (digitalRead(TinyArcadePin2) ? 0 : 1);
        }
        else {
            uint8_t rVal = 0;
            position.x = (digitalRead(TinyArcadePinLeft) ? FixedNumber16<4>(0,12) : 0) + (digitalRead(TinyArcadePinRight) ? FixedNumber16<4>(-1,4) : 0);
            position.y = (digitalRead(TinyArcadePinUp) ? FixedNumber16<4>(0,12) : 0) + (digitalRead(TinyArcadePinDown) ? FixedNumber16<4>(-1,4) : 0);
            buttonA = buttonA<<1 | (digitalRead(TinyArcadePin1) ? 0 : 1);
            buttonB = buttonB<<1 | (digitalRead(TinyArcadePin2) ? 0 : 1);
        }
    }
    bool getButton(int id) {
        return (id == 1 ? buttonB : buttonA)&1;
    }
    bool getButton(int id, int phase) {
        return ((id == 1 ? buttonB : buttonA)>>phase) & 1;
    }
    Fixed2D4 getJoystick() {
        return position;
    }
}
