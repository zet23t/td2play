#include "lib_math.h"

namespace Math {
    static unsigned int a = 0,b = 0;
    void setSeed(int na, int nb) {
        a = na, b = nb;
    }
    void getSeed(int& na, int& nb) {
        na = a, nb = b;
    }
    unsigned int randInt() {
        // This is my own random number generator that is, supposedly, very simple.
        // Obviously, this won't be used for anything fancy - just generating random
        // numbers for games should be fine. The used operations require only one CPU
        // cycle each - so it should be below 20 cycles in total.
        // The test code checks if a number range is uniformly filled and a visual 2d
        // diagram looks OK too.
        a = a - b + 238841239;
        b = a << 1 ^ b >> 1;
        return a;
    }
}
