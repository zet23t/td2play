#ifndef __LIB_MATH_H__
#define __LIB_MATH_H__

#include <inttypes.h>

namespace Math {
    void setSeed(int na, int nb);
    unsigned int randInt();
    void getSeed(int& na, int& nb);

    class Vector2D16 {
    public:
        int16_t x, y;
        Vector2D16(): x(0), y(0){};
        Vector2D16(int16_t x, int16_t y): x(x), y(y) {};
        Vector2D16 operator +(const Vector2D16& b) const {
            return Vector2D16(x + b.x, y + b.y);
        }

    };
}

#endif // __LIB_MATH_H__
