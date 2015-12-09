#ifndef __FIXEDMATH_H__
#define __FIXEDMATH_H__

template <uint8_t shiftNum>
class Fixed {
private:
    int16_t number;
public:
    Fixed* setNumber(int16_t full, int16_t frac) {
        number = full << shiftNum | (frac & ((1<<shiftNum) - 1));
    }

};

#endif // __FIXEDMATH_H__
