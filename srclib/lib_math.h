#ifndef __LIB_MATH_H__
#define __LIB_MATH_H__

namespace Math {
    class Vector2D16 {
    public:
        int16_t x, y;
        Vector2D16(): x(0), y(0){};
        Vector2D16(int16_t x, int16_t y): x(x), y(y) {};


    };
}

#endif // __LIB_MATH_H__
