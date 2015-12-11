#include "lib_FixedMath.h"
#include <assert.h>
#include <stdio.h>

#define F4 FixedNumber16<4>

class TestFixedMath {
public:
    TestFixedMath() {
        testCompares();
        testAddition();
        testSubtraction();
        testMultiplication();
        testDivision();
    }

    void testCompares() {
        assert(F4(7,7) != F4(9,3));
        assert(F4(23,2) == F4(23,2));
    }

    void testAddition() {
        assert(F4(16,10) == F4(7,7) + F4(9,3));
    }

    void testSubtraction() {
        assert(F4(1,12) == F4(9,3) - F4(7,7));
        assert(F4(-2,4) == F4(7,7) - F4(9,3));
    }

    void testMultiplication() {
        assert(F4(1,0) == F4(0,8) * F4(2,0));
        assert(F4() == F4(0,0) * F4(2,0));
    }

    void testDivision() {
        assert(F4(1,0) == F4(1,0) / F4(1,0));
        assert(F4(2,0) == F4(1,0) / F4(0,8));
    }

};
