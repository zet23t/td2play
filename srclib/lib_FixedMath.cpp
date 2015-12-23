#include "lib_FixedMath.h"
#include "lib_math.h"
template<uint8_t shiftNum>
void FixedNumber16<shiftNum>::random(const FixedNumber16<shiftNum>& min, const FixedNumber16<shiftNum>& max) {
    const FixedNumber16<shiftNum> range = max - min;
    number = Math::randInt();
    number%= range.number;
    number-= min.number;
}

void Fixed2D4::randomCircle(const FixedNumber16<4>& radius) {
    FixedNumber16<4> r2 = radius * radius;
    while(1) {
        x.random(-radius,radius);
        y.random(-radius,radius);
        FixedNumber16<4> dist = x*x + y*y;
        if (dist <= r2) break;
    }
}
