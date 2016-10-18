#ifndef __FIXEDMATH_H__
#define __FIXEDMATH_H__
#include "lib_StringBuffer.h"
#include <inttypes.h>
#include "lib_StringBuffer.h"
#include <stdio.h>

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
    inline int16_t getRaw() const {
        return number;
    }
    inline uint16_t getFractionPart() const {
        return number & ((1<<shiftNum)-1);
    }
    inline int16_t getIntegerPart() const {
        return number >> shiftNum;
    }

    inline int16_t getRounded() const {
        return (number >> shiftNum) + ((number & ((1<<shiftNum)-1)) > 8 ? 1 : 0);
    }

    inline FixedNumber16<shiftNum>& setIntegerPart(int16_t x) {
        number = (x << shiftNum) | (number & ((1<<shiftNum) - 1));
        return *this;
    }
    inline FixedNumber16<shiftNum>& setFractionPart(int16_t x) {
        number = (number & ~((1<<shiftNum) - 1)) | (x & ((1<<shiftNum) - 1));
        return *this;
    }

    inline FixedNumber16<shiftNum> sign(FixedNumber16<shiftNum> len) const {
        if (number < 0) return -len;
        return len;
    }

    inline FixedNumber16<shiftNum> absolute() const {
        if (*this < FixedNumber16<shiftNum>(0,0)) return -*this;
        return *this;
    }

    FixedNumber16<shiftNum> sqrt() {
        if (number == 0 || *this == FixedNumber16<shiftNum>(1,0)) return *this;
        FixedNumber16<shiftNum> guess = FixedNumber16<shiftNum>(1,0);
        if (*this > FixedNumber16<shiftNum>(2,0)) guess = half();
        for (int i=0;i<=8;i+=1) {
            FixedNumber16<shiftNum> newGuess = ((*this / guess) + guess) / FixedNumber16<shiftNum>(2,0);
            if (guess == newGuess) break;
            guess = newGuess;
        }

        return guess;
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

    FixedNumber16<shiftNum> operator -() const {
        return FixedNumber16<shiftNum>(-number);
    }

    float asFloat() const {
        return (float)getIntegerPart() + (float)(getFractionPart()) / (float)(1<<shiftNum);
    }

    FixedNumber16<shiftNum> operator *(const FixedNumber16<shiftNum>& b) const {
        //return FixedNumber16<shiftNum>((int16_t)((int32_t)number * (int32_t)b.number >> shiftNum));
        // probably not optimal here, but I have trouble figuring out how to make multiplication
        // work both with negative & positive numbers behaving the same way when there are "rounding errors".
        // Thus I convert the numbers to positive numbers and apply the sign later on.
        int32_t an = number < 0 ? -number : number;
        int32_t bn = b.number < 0 ? -b.number : b.number;
        int16_t sig = (number < 0) ^ (b.number < 0) ? -1 : 1;
        return FixedNumber16<shiftNum>(sig * (int16_t)(((an * bn) >> shiftNum)));
    }

    FixedNumber16<shiftNum> operator *(const int& b) const {
        return *this * (const FixedNumber16<shiftNum>)(b<<shiftNum);
        //return FixedNumber16<shiftNum>((int16_t)((int32_t)number * (int32_t)b));
    }

    FixedNumber16<shiftNum>& operator *=(const FixedNumber16<shiftNum>& b) {
        //number = ((int16_t)((int32_t)number * (int32_t)b.number >> shiftNum));
        *this = *this * b;
        return *this;
    }

    FixedNumber16<shiftNum>& operator +=(const FixedNumber16<shiftNum>& b) {
        number = ((int16_t)((int32_t)number + (int32_t)b.number));
        return *this;
    }

    FixedNumber16<shiftNum>& operator -=(const FixedNumber16<shiftNum>& b) {
        number = ((int16_t)((int32_t)number - (int32_t)b.number));
        return *this;
    }

    FixedNumber16<shiftNum> operator /(const FixedNumber16<shiftNum>& b) const {
        if (b == 0) {
            return FixedNumber16<shiftNum>(0);
        }
        int32_t an = number < 0 ? -number : number;
        int32_t bn = b.number < 0 ? -b.number : b.number;
        int16_t sig = (number < 0) ^ (b.number < 0) ? -1 : 1;
        return FixedNumber16<shiftNum>(sig * (int16_t)((an << shiftNum) / bn));
    }

    bool operator ==(const FixedNumber16<shiftNum>& b) const {
        return number == b.number;
    }

    bool operator !=(const FixedNumber16<shiftNum>& b) const {
        return number != b.number;
    }

    bool operator <(const FixedNumber16<shiftNum>& b) const {
        return number < b.number;
    }
    bool operator >(const FixedNumber16<shiftNum>& b) const {
        return number > b.number;
    }
    bool operator <=(const FixedNumber16<shiftNum>& b) const {
        return number <= b.number;
    }
    bool operator >=(const FixedNumber16<shiftNum>& b) const {
        return number >= b.number;
    }
    void random(const FixedNumber16<shiftNum>& min, const FixedNumber16<shiftNum>& max);
    FixedNumber16<shiftNum> half() const {
        return (FixedNumber16<shiftNum>)(number >> 1);
    }
};


class Fixed2D4 {
public:
    FixedNumber16<4> x, y;
    Fixed2D4():x(0),y(0) {

    }
    Fixed2D4(int16_t intX, int16_t intY):x(intX,0),y(intY,0) {
    }
    Fixed2D4(int16_t intX, uint16_t fracX, int16_t intY, uint16_t fracY):
        x(intX,fracX),y(intY,fracY) {
    }
    Fixed2D4(FixedNumber16<4> fx, FixedNumber16<4> fy):
        x(fx),y(fy) {
    }

    Fixed2D4& setXY(int16_t intX, int16_t intY) {
        x.setNumber(intX,0);
        y.setNumber(intY,0);
        return *this;
    }

    Fixed2D4& scale(const Fixed2D4& b) {
        x *= b.x;
        y *= b.y;
        return *this;
    }

    Fixed2D4& scale(const FixedNumber16<4>& b) {
        x *= b;
        y *= b;
        return *this;
    }

    Fixed2D4& setIntegerPart(const Fixed2D4& b) {
        x.setIntegerPart(b.x.getIntegerPart());
        y.setIntegerPart(b.y.getIntegerPart());
        return *this;
    }

    Fixed2D4& scale(int16_t integer, int16_t frac) {
        FixedNumber16<4> n = FixedNumber16<4>(integer,frac);
        x *= n;
        y *= n;
        return *this;
    }

    FixedNumber16<4> length() const {
        FixedNumber16<4> sqd = x * x + y * y;
        return sqd.sqrt();
    }

    FixedNumber16<4> sqLength() const {
        return x * x + y * y;
    }

    Fixed2D4& normalize() {
        bool xeq = x == FixedNumber16<4>(0,0);
        bool yeq = y == FixedNumber16<4>(0,0);

        //if (y > 0) printf("%f %f\n",(x/y).asFloat(),y.asFloat());
        if (xeq && yeq) {
            return *this;
        }
        if (xeq) {
            x = 0;
            y = FixedNumber16<4>(y < 0 ? -1 : 1,0);
            return *this;
        }
        if (yeq) {
            y = 0;
            x = FixedNumber16<4>(x < 0 ? -1 : 1,0);
            return *this;
        }
        FixedNumber16<4> sqd = x * x + y * y;
        FixedNumber16<4> len = sqd.sqrt();// - FixedNumber16<4>(0,1);
        x = x / len;
        y = y / len;
        if (y == 0) x = FixedNumber16<4>(x < 0 ? -1 : 1,0);
        if (x == 0) y = FixedNumber16<4>(y < 0 ? -1 : 1,0);
        FixedNumber16<4> sqlen = calcSqrLen();

        if (sqlen > FixedNumber16<4>(1,0)) {
            FixedNumber16<4> s = sqlen - FixedNumber16<4>(1,0);
            if (x.absolute() > y.absolute()) y += (y < 0 ? s : -s);
            else x += (x < 0 ? s : -s);
        }
        else if (sqlen < FixedNumber16<4>(1,0)) {
            FixedNumber16<4> s = FixedNumber16<4>(1,0) - sqlen;
            if (x.absolute() > y.absolute()) y -= (y < 0 ? s : -s);
            else x -= (x < 0 ? s : -s);
        }
        return *this;
    }

    FixedNumber16<4> dot(const Fixed2D4& b) const {
        return x * b.x + y * b.y;
    }

    Fixed2D4 operator +(const Fixed2D4& b) const {
        return Fixed2D4(x + b.x, y + b.y);
    }

    Fixed2D4 operator *(const int& b) const {
        return Fixed2D4(x * b, y * b);
    }

    Fixed2D4 left () const {
        return Fixed2D4(-y,x);
    }

    Fixed2D4 right() const {
        return Fixed2D4(y,-x);
    }


    Fixed2D4 operator *(const FixedNumber16<4>& b) const {
        return Fixed2D4(x * b, y * b);
    }

    Fixed2D4& operator += (const Fixed2D4& b) {
        x += b.x;
        y += b.y;
        return *this;
    }

    Fixed2D4& operator -= (const Fixed2D4& b) {
        x -= b.x;
        y -= b.y;
        return *this;
    }

    Fixed2D4 operator -(const Fixed2D4& b) const {
        return Fixed2D4(x - b.x, y - b.y);
    }

    Fixed2D4 operator -() const {
        return Fixed2D4(-x, -y);
    }

    Fixed2D4 randomCircle(const FixedNumber16<4>& radius) const;

    bool operator ==(const Fixed2D4& b) const {
        return x == b.x && y == b.y;
    }
    bool operator !=(const Fixed2D4& b) const {
        return x != b.x || y != b.y;
    }

    FixedNumber16<4> calcSqrLen() const {
        return (x * x) + (y * y);
    }
};

#endif // __FIXEDMATH_H__
