#include "lib_FixedMath.h"
#include <assert.h>

class TestFixedMath {
public:
    TestFixedMath() {
        testAddition();
    }

    void testAddition() {
        FixedNumber16<4> a(7,7);
        FixedNumber16<4> b(9,3);
        FixedNumber16<4> expected(16,10);
        FixedNumber16<4> notExpected(16,11);
        assert(expected == a + b);
        assert(notExpected != a + b);
    }

};
