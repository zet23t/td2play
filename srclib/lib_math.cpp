#include "lib_math.h"

namespace Math {
    static unsigned int a = 0,b = 0;
    void setSeed(int na, int nb) {
        a = na, b = nb;
    }
    unsigned int randInt() {
        //a = (b - 329487) ^ (b << 1) ^ (a >> 5) + b;
        //b = (32948121 - a) ^ (b << 4) ^ (a << 1) + a;
        a = a - b + 238841;
        b = a << 1 ^ b >> 1;
        return a ^ b;
    }
}
