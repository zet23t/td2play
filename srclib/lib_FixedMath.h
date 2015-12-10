#ifndef __FIXEDMATH_H__
#define __FIXEDMATH_H__
#include "lib_StringBuffer.h"

template <uint8_t shiftNum>
class FixedNumber {
private:
    int16_t number;
public:
    FixedNumber() {
    }
    FixedNumber(int16_t full, int16_t frac) {
        setNumber(full,frac);
    }
    FixedNumber& setNumber(int16_t full, int16_t frac) {
        number = full << shiftNum | (frac & ((1<<shiftNum) - 1));
        return *this;
    }
    inline uint16_t getFractionPart() {
        return number & ((1<<shiftNum)-1);
    }
    char* toString() {
        return toString(10000);
    }
    FixedNumber<shiftNum>& operator+(FixedNumber<shiftNum>& b) {
        // actual increment takes place here
        return *this;
    }
    char* toString(uint32_t fractionPrecission) {
        return stringBuffer.putDec(number >> shiftNum).put('.')
            .putDec((getFractionPart() * fractionPrecission) >> shiftNum ).get();
    }
};

#endif // __FIXEDMATH_H__
