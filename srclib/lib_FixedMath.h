#ifndef __FIXEDMATH_H__
#define __FIXEDMATH_H__
#include "lib_StringBuffer.h"

template <uint8_t shiftNum>
class FixedNumber16 {
private:
    int16_t number;
public:
    FixedNumber16() {
        number = 0;
    }
    FixedNumber16(uint16_t rawNumber) {
        number = rawNumber;
    }
    FixedNumber16(int16_t numerator, int16_t frac) {
        setNumber(numerator,frac);
    }
    FixedNumber16& setNumber(int16_t numerator, int16_t frac) {
        number = numerator << shiftNum | (frac & ((1<<shiftNum) - 1));
        return *this;
    }
    inline uint16_t getFractionPart() const {
        return number & ((1<<shiftNum)-1);
    }
    char* toString() const {
        return toString(10000);
    }
    char* toString(uint32_t fractionPrecission) const {
        return stringBuffer.putDec(number >> shiftNum).put('.')
            .putDec((getFractionPart() * fractionPrecission) >> shiftNum ).get();
    }

    FixedNumber16<shiftNum> operator +(const FixedNumber16<shiftNum>& b) const {
        return FixedNumber16<shiftNum>(number + b.number);
    }

    FixedNumber16<shiftNum> operator -(const FixedNumber16<shiftNum>& b) const {
        return FixedNumber16<shiftNum>(number - b.number);
    }

    FixedNumber16<shiftNum> operator *(const FixedNumber16<shiftNum>& b) const {
        return FixedNumber16<shiftNum>((int16_t)((int32_t)number * (int32_t)b.number >> shiftNum));
    }

    FixedNumber16<shiftNum> operator /(const FixedNumber16<shiftNum>& b) const {
        return FixedNumber16<shiftNum>((int16_t)(((int32_t)number << shiftNum) / (int32_t)b.number));
    }

    bool operator ==(const FixedNumber16<shiftNum>& b) const {
        return number == b.number;
    }

    bool operator !=(const FixedNumber16<shiftNum>& b) const {
        return number != b.number;
    }
};


class Fixed2D4 {
private:
    FixedNumber16<4> x, y;
public:
    Fixed2D4(int16_t intX, int16_t intY):x(intX,0),y(intY,0) {
    }

    bool operator ==(const Fixed2D4& b) const {
        return x == b.x && y == b.y;
    }
    bool operator !=(const Fixed2D4& b) const {
        return x != b.x || y != b.y;
    }
};

#endif // __FIXEDMATH_H__
